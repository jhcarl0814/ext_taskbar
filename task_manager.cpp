#include "quick_launch.hpp"

//#define NOMINMAX
#include <windows.h>

#include <ranges>
#include <windows.h>
#include <psapi.h>
#include <winternl.h>

#include <QStandardItemModel>

#include <QTreeView>
#include <QHeaderView>

#include "ext_infrastructure/ext_error_handling.hpp"
#include "ext_gui/ext_image.hpp"

std::optional<HANDLE> enable_token_privilege(LPCTSTR pszPrivilege)
{
    if(HANDLE hToken; OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) != 0)
    {
        bool succeeded;
        if(TOKEN_PRIVILEGES tkp{.PrivilegeCount = 1, .Privileges = {LUID_AND_ATTRIBUTES{.Attributes = SE_PRIVILEGE_ENABLED}}}; LookupPrivilegeValue(nullptr, pszPrivilege, &tkp.Privileges[0].Luid) != 0)
        {
            if(AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, static_cast<PTOKEN_PRIVILEGES>(nullptr), nullptr) != 0)
            {
                switch(DWORD lastError = GetLastError())
                {
                case ERROR_SUCCESS:
                    succeeded = true;
                    break;
                case ERROR_NOT_ALL_ASSIGNED:
                    ext_debug_log((pszPrivilege, hToken, lastError, GetLastErrorReturnValueToString(GetLastError(), "AdjustTokenPrivileges")), qDebug_compact());
                    succeeded = false;
                    break;
                default:
                    ext_debug_log((pszPrivilege, hToken, lastError, GetLastErrorReturnValueToString(GetLastError(), "AdjustTokenPrivileges")), qDebug_compact());
                    succeeded = false;
                    break;
                }
            }
            else
            {
                ext_debug_log((pszPrivilege, hToken, GetLastErrorReturnValueToString(GetLastError(), "AdjustTokenPrivileges")), qDebug_compact());
                succeeded = false;
            }
        }
        else
        {
            ext_debug_log((pszPrivilege, hToken, GetLastErrorReturnValueToString(GetLastError(), "LookupPrivilegeValue")), qDebug_compact());
            succeeded = false;
        }
        if(succeeded)
            return hToken;
        else
        {
            if(CloseHandle(hToken) == 0)
                ext_debug_log((hToken, GetLastErrorReturnValueToString(GetLastError(), "CloseHandle")), qDebug_compact());
            return std::nullopt;
        }
    }
    else
    {
        ext_debug_log((pszPrivilege, GetLastErrorReturnValueToString(GetLastError(), "OpenProcessToken")), qDebug_compact());
        return std::nullopt;
    }
}

typedef NTSTATUS(NTAPI *pfnNtQueryInformationProcess)(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL);
pfnNtQueryInformationProcess gNtQueryInformationProcess; // https://www.codeproject.com/Articles/19685/Get-Process-Info-with-NtQueryInformationProcess
std::optional<HMODULE> load_NtQueryInformationProcess()
{
    // Load NTDLL Library and get entry address for NtQueryInformationProcess
    if(HMODULE hNtDll = LoadLibrary(TEXT("ntdll.dll")); hNtDll != NULL)
    {
        if(gNtQueryInformationProcess = (pfnNtQueryInformationProcess)GetProcAddress(hNtDll, "NtQueryInformationProcess"); gNtQueryInformationProcess != NULL)
            return hNtDll;
        else
        {
            ext_debug_log((hNtDll, GetLastErrorReturnValueToString(GetLastError(), "GetProcAddress")), qDebug_compact());
            if(FreeLibrary(hNtDll) == 0)
                ext_debug_log((hNtDll, GetLastErrorReturnValueToString(GetLastError(), "FreeLibrary")), qDebug_compact());
            return std::nullopt;
        }
    }
    else
    {
        ext_debug_log((GetLastErrorReturnValueToString(GetLastError(), "LoadLibrary")), qDebug_compact());
        return std::nullopt;
    }
}
void free_NtQueryInformationProcess(HMODULE hNtDll)
{
    if(FreeLibrary(hNtDll) == 0)
        ext_debug_log((hNtDll, GetLastErrorReturnValueToString(GetLastError(), "FreeLibrary")), qDebug_compact());
    gNtQueryInformationProcess = NULL;
}

void create_widget()
{
    struct module_info_t
    {
        std::optional<std::basic_string<TCHAR>> module_base_name;
        std::optional<std::basic_string<TCHAR>> module_file_name;
    };
    struct process_info_t
    {
        std::optional<std::basic_string<TCHAR>> process_image_file_name;
        std::optional<std::basic_string<TCHAR>> full_process_image_name_win32_path_format;
        std::optional<std::basic_string<TCHAR>> full_process_image_name_native_system_path_format;
        std::map<DWORD, module_info_t> module_infos;
        ULONG_PTR unique_process_id;
        ULONG_PTR inherited_from_unique_process_id;
        std::optional<std::basic_string<TCHAR>> image_path_name;
        std::optional<std::basic_string<TCHAR>> command_line;
    };
    std::map<DWORD, std::optional<process_info_t>> process_infos;
    if(std::optional<HANDLE> opt_access_token_handle = enable_token_privilege(SE_DEBUG_NAME); opt_access_token_handle.has_value())
    {
        if(std::optional<HMODULE> opt_hNtDll = load_NtQueryInformationProcess(); opt_hNtDll.has_value())
        {
            if(DWORD process_ids[1024], process_count; EnumProcesses(process_ids, sizeof(process_ids), &process_count) != 0) // https://learn.microsoft.com/en-us/windows/win32/psapi/enumerating-all-processes
            {
                for(auto process_id : process_ids | std::views::take(process_count / sizeof(DWORD)) | std::views::filter([](DWORD const &process_id)
                                                                                                          { return process_id != 0; }))
                {
                    if(HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id); process_handle != NULL)
                    {
                        process_info_t process_info;
                        {
                            TCHAR process_image_file_name[MAX_PATH]{};
                            if(DWORD process_image_file_name_length = GetProcessImageFileName(process_handle, process_image_file_name, sizeof(process_image_file_name) / sizeof(TCHAR)); process_image_file_name_length != 0)
                                process_info.process_image_file_name.emplace(process_image_file_name, process_image_file_name_length);
                            else
                                ext_debug_log((opt_access_token_handle.value(), opt_hNtDll.value(), process_id, process_handle, process_image_file_name_length, GetLastErrorReturnValueToString(GetLastError(), "GetProcessImageFileName")), qDebug_compact());

                            TCHAR full_process_image_name_win32_path_format[MAX_PATH]{};
                            DWORD full_process_image_name_win32_path_format_length = sizeof(full_process_image_name_win32_path_format) / sizeof(TCHAR);
                            if(QueryFullProcessImageName(process_handle, 0, full_process_image_name_win32_path_format, &full_process_image_name_win32_path_format_length) != 0)
                                process_info.full_process_image_name_win32_path_format.emplace(full_process_image_name_win32_path_format, full_process_image_name_win32_path_format_length);
                            else
                                ext_debug_log((opt_access_token_handle.value(), opt_hNtDll.value(), process_id, process_handle, GetLastErrorReturnValueToString(GetLastError(), "QueryFullProcessImageName")), qDebug_compact());

                            TCHAR full_process_image_name_native_system_path_format[MAX_PATH]{};
                            DWORD full_process_image_name_native_system_path_format_length = sizeof(full_process_image_name_native_system_path_format) / sizeof(TCHAR);
                            if(QueryFullProcessImageName(process_handle, PROCESS_NAME_NATIVE, full_process_image_name_native_system_path_format, &full_process_image_name_native_system_path_format_length) != 0)
                                process_info.full_process_image_name_native_system_path_format.emplace(full_process_image_name_native_system_path_format, full_process_image_name_native_system_path_format_length);
                            else
                                ext_debug_log((opt_access_token_handle.value(), opt_hNtDll.value(), process_id, process_handle, GetLastErrorReturnValueToString(GetLastError(), "QueryFullProcessImageName")), qDebug_compact());
                        }

                        {
                            HMODULE module_handles[1024];
                            DWORD module_count;
                            if(EnumProcessModules(process_handle, module_handles, sizeof(module_handles), &module_count) != 0) // https://learn.microsoft.com/en-us/windows/win32/psapi/enumerating-all-modules-for-a-process
                            {
                                for(DWORD module_index : std::views::iota(static_cast<DWORD>(0), module_count / sizeof(HMODULE)))
                                {
                                    module_info_t module_info;

                                    TCHAR module_base_name[MAX_PATH]{};
                                    if(DWORD module_base_name_length = GetModuleBaseName(process_handle, module_handles[module_index], module_base_name, sizeof(module_base_name) / sizeof(TCHAR)); module_base_name_length != 0)
                                        module_info.module_base_name.emplace(module_base_name, module_base_name_length);
                                    else
                                        ext_debug_log((opt_access_token_handle.value(), opt_hNtDll.value(), process_id, process_handle, module_index, module_handles[module_index], module_base_name_length, GetLastErrorReturnValueToString(GetLastError(), "GetModuleBaseName")), qDebug_compact());

                                    TCHAR module_file_name[MAX_PATH]{};
                                    if(DWORD module_file_name_length = GetModuleFileNameEx(process_handle, module_handles[module_index], module_file_name, sizeof(module_file_name) / sizeof(TCHAR)); module_file_name_length != 0)
                                        module_info.module_file_name.emplace(module_file_name, module_file_name_length);
                                    else
                                        ext_debug_log((opt_access_token_handle.value(), opt_hNtDll.value(), process_id, process_handle, module_index, module_handles[module_index], module_file_name_length, GetLastErrorReturnValueToString(GetLastError(), "GetModuleFileNameEx")), qDebug_compact());

                                    process_info.module_infos.emplace(std::piecewise_construct, std::forward_as_tuple(module_index), std::forward_as_tuple(module_info));
                                }
                            }
                            else
                                ext_debug_log((opt_access_token_handle.value(), opt_hNtDll.value(), process_id, process_handle, GetLastErrorReturnValueToString(GetLastError(), "EnumProcessModules")), qDebug_compact());
                        }

                        {
                            typedef struct _PROCESS_BASIC_INFORMATION
                            {
                                NTSTATUS ExitStatus;
                                PPEB PebBaseAddress;
                                ULONG_PTR AffinityMask;
                                KPRIORITY BasePriority;
                                ULONG_PTR UniqueProcessId;
                                ULONG_PTR InheritedFromUniqueProcessId;
                            } PROCESS_BASIC_INFORMATION;
                            PROCESS_BASIC_INFORMATION pbi;
                            if(NTSTATUS dwStatus = gNtQueryInformationProcess(process_handle, PROCESSINFOCLASS::ProcessBasicInformation, &pbi, sizeof(PROCESS_BASIC_INFORMATION), nullptr); dwStatus >= 0) // https://www.osr.com/blog/2020/04/23/ntstatus-to-win32-error-code-mappings/
                            {
                                process_info.unique_process_id = pbi.UniqueProcessId;
                                process_info.inherited_from_unique_process_id = pbi.InheritedFromUniqueProcessId;
                                if(pbi.PebBaseAddress)
                                {
                                    PEB peb;
                                    if(ReadProcessMemory(process_handle, pbi.PebBaseAddress, &peb, sizeof(peb), NULL) != 0)
                                    {
                                        RTL_USER_PROCESS_PARAMETERS process_parameters;
                                        if(ReadProcessMemory(process_handle, peb.ProcessParameters, &process_parameters, sizeof(process_parameters), NULL) != 0)
                                        {
                                            TCHAR image_path_name[MAX_PATH]{};
                                            SIZE_T image_path_name_length;
                                            if(ReadProcessMemory(process_handle, process_parameters.ImagePathName.Buffer, image_path_name, process_parameters.ImagePathName.Length, &image_path_name_length) != 0)
                                                process_info.image_path_name.emplace(image_path_name, process_parameters.ImagePathName.Length / sizeof(TCHAR));
                                            else
                                                ext_debug_log((opt_access_token_handle.value(), opt_hNtDll.value(), process_id, process_handle, dwStatus, GetLastErrorReturnValueToString(GetLastError(), "ReadProcessMemory")), qDebug_compact());
                                            TCHAR command_line[8192]{};
                                            SIZE_T command_line_length;
                                            if(ReadProcessMemory(process_handle, process_parameters.CommandLine.Buffer, command_line, process_parameters.CommandLine.Length, &command_line_length) != 0)
                                                process_info.command_line.emplace(command_line, process_parameters.CommandLine.Length / sizeof(TCHAR));
                                            else
                                                ext_debug_log((opt_access_token_handle.value(), opt_hNtDll.value(), process_id, process_handle, dwStatus, GetLastErrorReturnValueToString(GetLastError(), "ReadProcessMemory")), qDebug_compact());
                                        }
                                        else
                                            ext_debug_log((opt_access_token_handle.value(), opt_hNtDll.value(), process_id, process_handle, dwStatus, GetLastErrorReturnValueToString(GetLastError(), "ReadProcessMemory")), qDebug_compact());
                                    }
                                    else
                                        ext_debug_log((opt_access_token_handle.value(), opt_hNtDll.value(), process_id, process_handle, dwStatus, GetLastErrorReturnValueToString(GetLastError(), "ReadProcessMemory")), qDebug_compact());
                                }
                            }
                            else
                                ext_debug_log((opt_access_token_handle.value(), opt_hNtDll.value(), process_id, process_handle, dwStatus, GetLastErrorReturnValueToString(GetLastError(), "NtQueryInformationProcess")), qDebug_compact());
                        }

                        if(CloseHandle(process_handle) == 0)
                            ext_debug_log((opt_access_token_handle.value(), opt_hNtDll.value(), process_id, process_handle, GetLastErrorReturnValueToString(GetLastError(), "CloseHandle")), qDebug_compact());
                        process_infos.emplace(std::piecewise_construct, std::forward_as_tuple(process_id), std::forward_as_tuple(std::move(process_info)));
                    }
                    else
                    {
                        ext_debug_log((opt_access_token_handle.value(), opt_hNtDll.value(), process_id, process_handle, GetLastErrorReturnValueToString(GetLastError(), "OpenProcess")), qDebug_compact());
                        process_infos.emplace(std::piecewise_construct, std::forward_as_tuple(process_id), std::forward_as_tuple(std::nullopt));
                    }
                }
            }
            else
                ext_debug_log((opt_access_token_handle.value(), opt_hNtDll.value(), GetLastErrorReturnValueToString(GetLastError(), "EnumProcesses")), qDebug_compact());

            free_NtQueryInformationProcess(opt_hNtDll.value());
        }
        if(CloseHandle(opt_access_token_handle.value()) == 0)
            ext_debug_log((opt_access_token_handle.value(), GetLastErrorReturnValueToString(GetLastError(), "CloseHandle")), qDebug_compact());
    }



    {
        using namespace Qt::Literals::StringLiterals;

        QTreeView *tree_view = new QTreeView();
        QStandardItemModel *model = new QStandardItemModel();
        model->setHorizontalHeaderLabels({
            u"process_id"_s + u"\n"_s +
                u"modules[0]'s base_name"_s,
            u"process_image_file_name"_s + u"\n"_s +
                u"full_process_image_name_win32_path_format"_s + u"\n"_s +
                u"full_process_image_name_native_system_path_format"_s,
            //            u"unique_process_id"_s,
            //            u"inherited_from_unique_process_id"_s,
            u"image_path_name"_s + u"\n"_s +
                u"command_line"_s,
        });

        struct process_node_t
        {
            std::optional<process_info_t *> opt_p_process_info;
            std::map<DWORD, process_node_t> children;
        };
        std::map<DWORD, process_node_t> root_process_nodes;
        std::map<DWORD, process_node_t *> process_id_to_p_process_node;
        auto create_node_if_not_exists = [&](auto &this_, DWORD process_id, std::optional<process_info_t> &opt_process_info) -> process_node_t *
        {
            if(auto it_id_to_p_node = process_id_to_p_process_node.find(process_id); it_id_to_p_node == process_id_to_p_process_node.end()) //not exists
            {
                std::map<DWORD, process_node_t> *map_to_emplace;
                if(!opt_process_info.has_value())
                    map_to_emplace = &root_process_nodes;
                else
                {
                    auto process_info = opt_process_info.value();
                    process_node_t *p_parent_node;
                    auto it_parent_process_info = process_infos.find(process_info.inherited_from_unique_process_id);
                    p_parent_node = this_(this_, process_info.inherited_from_unique_process_id, it_parent_process_info != process_infos.end() ? it_parent_process_info->second : unmove(std::optional<process_info_t>(std::nullopt)));
                    map_to_emplace = &p_parent_node->children;
                }
                auto [it, succeeded] = map_to_emplace->emplace(std::piecewise_construct, std::forward_as_tuple(process_id), std::forward_as_tuple(process_node_t{.opt_p_process_info = opt_process_info.transform([](process_info_t &process_info)
                                                                                                                                                                     { return &process_info; })}));
                process_id_to_p_process_node.emplace(std::piecewise_construct, std::forward_as_tuple(process_id), std::forward_as_tuple(&it->second));
                return &it->second;
            }
            else
                return it_id_to_p_node->second;
        };
        for(auto &[process_id, opt_process_info] : process_infos)
            create_node_if_not_exists(create_node_if_not_exists, process_id, opt_process_info);

        auto append_child = [](auto &this_, DWORD process_id, process_node_t &process_node, QStandardItem *item) -> void
        {
            QList<QStandardItem *> items;
            QStandardItem *item_id = new QStandardItem([&]()
                {
                if(process_node.opt_p_process_info.has_value()&&
                        process_node.opt_p_process_info.value()->module_infos.size()>=1&&
                        process_node.opt_p_process_info.value()->module_infos.begin()->second.module_base_name.has_value())
                {
                    return QString::number(process_id) + u" "_s +
                            QString::fromStdWString(process_node.opt_p_process_info.value()->module_infos.begin()->second.module_base_name.value());
                }
                else
                    return QString::number(process_id); }());
            if(process_node.opt_p_process_info.has_value() &&
                process_node.opt_p_process_info.value()->full_process_image_name_win32_path_format.has_value())
            {
                QString file_path = QString::fromStdWString(process_node.opt_p_process_info.value()->full_process_image_name_win32_path_format.value());
                if(auto optional_icon = filePath_to_QIcon_gui(file_path).or_else([&]
                       { return filePath_to_QIcon_widgets(file_path); });
                    optional_icon.has_value())
                {
                    QIcon icon = *optional_icon;
                    item_id->setIcon(icon);
                }
            }
            items.push_back(item_id);
            if(process_node.opt_p_process_info.has_value())
            {
                auto &process_info = *process_node.opt_p_process_info.value();
                items.push_back(new QStandardItem(QString::fromStdWString(process_info.process_image_file_name.value_or(TEXT(""))) + u"\n"_s +
                    QString::fromStdWString(process_info.full_process_image_name_win32_path_format.value_or(TEXT(""))) + u"\n"_s +
                    QString::fromStdWString(process_info.full_process_image_name_native_system_path_format.value_or(TEXT("")))));
                //                items.push_back(new QStandardItem(QString::number(process_info.unique_process_id)));
                //                items.push_back(new QStandardItem(QString::number(process_info.inherited_from_unique_process_id)));
                items.push_back(new QStandardItem(QString::fromStdWString(process_info.image_path_name.value_or(TEXT(""))) + u"\n"_s +
                    QString::fromStdWString(process_info.command_line.value_or(TEXT("")))));
            }
            for(auto item : items)
                item->setTextAlignment(Qt::AlignmentFlag::AlignTop);
            item_id->setTextAlignment(Qt::AlignmentFlag::AlignVCenter);
            for(auto &[child_process_id, child_process_node] : process_node.children)
            {
                this_(this_, child_process_id, child_process_node, items[0]);
            }
            item->appendRow(items);
        };
        for(auto &[process_id, process_node] : root_process_nodes)
            append_child(append_child, process_id, process_node, model->invisibleRootItem());

        if(false)
        {
            for(auto &[process_id, opt_process_info] : process_infos)
            {
                QList<QStandardItem *> items;
                items.push_back(new QStandardItem(QString::number(process_id)));
                if(opt_process_info.has_value())
                {
                    auto &process_info = opt_process_info.value();
                    items.push_back(new QStandardItem(QString::fromStdWString(process_info.process_image_file_name.value_or(TEXT(""))) + u"\n"_s +
                        QString::fromStdWString(process_info.full_process_image_name_win32_path_format.value_or(TEXT(""))) + u"\n"_s +
                        QString::fromStdWString(process_info.full_process_image_name_native_system_path_format.value_or(TEXT("")))));
                    //                    items.push_back(new QStandardItem(QString::number(process_info.unique_process_id)));
                    //                    items.push_back(new QStandardItem(QString::number(process_info.inherited_from_unique_process_id)));
                    items.push_back(new QStandardItem(QString::fromStdWString(process_info.image_path_name.value_or(TEXT(""))) + u"\n"_s +
                        QString::fromStdWString(process_info.command_line.value_or(TEXT("")))));
                }
                for(auto item : items)
                {
                    item->setTextAlignment(Qt::AlignmentFlag::AlignTop);
                }
                model->appendRow(items);
            }
        }
        tree_view->setModel(model);
        tree_view->expandAll();
        tree_view->header()->setStretchLastSection(false);
        tree_view->header()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);

        tree_view->setAlternatingRowColors(true);
        tree_view->show();
    }
}
