#include "quick_launch.hpp"

//#define NOMINMAX
#include <windows.h>

#include "ext_infrastructure/ext_debug.hpp"
#include "ext_infrastructure/map_default_value.hpp"
#include "ext_infrastructure/overloaded.hpp"
#include "ext_infrastructure/ext_configs.hpp"
#include "ext_infrastructure/ext_algorithm.hpp"
#include "ext_infrastructure/ext_thread.hpp"

#include <QBuffer>
#include <QSaveFile>
#include <QJsonDocument>
#include <QMimeData>
#include "ext_core/ext_dir_iterator.hpp"
#include "ext_core/ext_file_info.hpp"
#include "ext_core/ext_thread.hpp"

#include <QGuiApplication>
#include <QClipBoard>
#include "ext_gui/ext_image.hpp"

#include <QTabWidget>
#include <QToolButton>
#include <QPushButton>
#include "ext_widgets/flow_layout.hpp"
#include "ext_widgets/scroll_area_vertical.hpp"
#include "ext_widgets/menu_permanent.hpp"
#include "ext_widgets/menu_window.hpp"
#include "ext_widgets/menu_positioned.hpp"
#include "ext_widgets/ext_text_edit.hpp"
#include "ext_widgets/ext_groupbox.hpp"
#include "ext_widgets/escape_ampersand.hpp"

std::map<int, QString> const quick_launch_button_t::shell_execute_t::window_show_state_code_to_name{{
    {0, "SW_HIDE"},
    {1, "SW_SHOWNORMAL"},
    {2, "SW_SHOWMINIMIZED"},
    {3, "SW_SHOWMAXIMIZED"},
    {4, "SW_SHOWNOACTIVATE"},
    {5, "SW_SHOW"},
    {6, "SW_MINIMIZE"},
    {7, "SW_SHOWMINNOACTIVE"},
    {8, "SW_SHOWNA"},
    {9, "SW_RESTORE"},
    {10, "SW_SHOWDEFAULT"},
    {11, "SW_FORCEMINIMIZE"},
}};
std::map<QString, int> const quick_launch_button_t::shell_execute_t::window_show_state_name_to_code{{
    {"SW_HIDE", 0},
    {"SW_SHOWNORMAL", 1},
    {"SW_NORMAL", 1},
    {"SW_SHOWMINIMIZED", 2},
    {"SW_SHOWMAXIMIZED", 3},
    {"SW_MAXIMIZE", 3},
    {"SW_SHOWNOACTIVATE", 4},
    {"SW_SHOW", 5},
    {"SW_MINIMIZE", 6},
    {"SW_SHOWMINNOACTIVE", 7},
    {"SW_SHOWNA", 8},
    {"SW_RESTORE", 9},
    {"SW_SHOWDEFAULT", 10},
    {"SW_FORCEMINIMIZE", 11},
}};

QJsonObject quick_launch_button_t::shell_execute_t::to_json_impl() const
{
    return QJsonObject({
        {"verb", std_optional_to_QJsonValue(verb)},
        {"file_path", file_path},
        {"parameters", std_optional_to_QJsonValue(parameters)},
        {"working_directory", std_optional_to_QJsonValue(working_directory)},
        {"window_show_state", get1(window_show_state_code_to_name, window_show_state, "SW_SHOWDEFAULT")},
    });
}
quick_launch_button_t::shell_execute_t quick_launch_button_t::shell_execute_t::from_json_impl(QJsonObject &obj)
{
    return shell_execute_t{
        .verb = QJsonValue_to_std_optional<decltype(verb)>(obj.take("verb")),
        .file_path = obj.take("file_path").toString(),
        .parameters = QJsonValue_to_std_optional<decltype(parameters)>(obj.take("parameters")),
        .working_directory = QJsonValue_to_std_optional<decltype(working_directory)>(obj.take("working_directory")),
        .window_show_state = get1(window_show_state_name_to_code, obj.take("window_show_state").toString(), window_show_state_name_to_code.at("SW_SHOWDEFAULT")),
    };
}

QJsonObject quick_launch_button_t::create_process_t::to_json_impl() const
{
    return QJsonObject({
        {"command_line", command_line},
        {"environment_block", std_optional_to_QJsonValue(environment_block)},
        {"working_directory", std_optional_to_QJsonValue(working_directory)},
        {"title", std_optional_to_QJsonValue(title)},
    });
}
quick_launch_button_t::create_process_t quick_launch_button_t::create_process_t::from_json_impl(QJsonObject &obj)
{
    return create_process_t{
        .command_line = obj.take("command_line").toString(),
        .environment_block = QJsonValue_to_std_optional<decltype(environment_block)>(obj.take("environment_block")),
        .working_directory = QJsonValue_to_std_optional<decltype(working_directory)>(obj.take("working_directory")),
        .title = QJsonValue_to_std_optional<decltype(title)>(obj.take("title")),
    };
}

QJsonObject quick_launch_button_t::to_json_impl() const
{
    QJsonObject result({
        {"collapsed", collapsed},
    });
    std_variant_to_QJsonObject_member(
        result, shell_execute_or_create_process, type_list_t<shell_execute_t, create_process_t>{}, value_list_t<template_str_t("shell_execute"), template_str_t("create_process")>{});
    result["display_text"] = optionally_overriding_to_QJsonValue(display_text_overriding, [&]
        { return display_text; });
    result["icons"] = optionally_overriding_to_QJsonValue(icons_overriding, [&]
        { return icons | std::views::transform(&QImage_to_QByteArray) | std::views::transform([](QByteArray const &ba)
                                                                            { return ba.toBase64(); }) |
              std::views::transform(qOverload<QByteArrayView>.operator()<QString>(&QString::fromLatin1)) | std::ranges::to<QJsonArray>(); });
    return result;
}
quick_launch_button_t quick_launch_button_t::from_json_impl(QJsonObject &obj)
{
    quick_launch_button_t quick_launch_button{
        .collapsed = obj.take("collapsed").toBool(),
    };
    quick_launch_button.shell_execute_or_create_process = QJsonObject_member_to_std_variant(obj, type_list_t<decltype(shell_execute_or_create_process)>(), value_list_t<template_str_t("shell_execute"), template_str_t("create_process")>());
    std::tie(quick_launch_button.display_text_overriding, quick_launch_button.display_text) = QJsonValue_to_optionally_overriding(obj.take("display_text"), [&]
        { return std::visit(overloaded{[&](shell_execute_t &shell_execute)
                                {
                                    QFileInfo file_info(shell_execute.file_path);
                                    return file_info.baseName() != "" ? file_info.baseName() : file_info.absoluteFilePath();
                                },
                                [&](create_process_t &create_process)
                                {
                                    return create_process.title.value_or(create_process.command_line);
                                }},
              quick_launch_button.shell_execute_or_create_process); });
    std::tie(quick_launch_button.icons_overriding, quick_launch_button.icons) = QJsonValue_to_optionally_overriding(
        obj.take("icons"), [&]
        { return decltype(icons){}; },
        [](QJsonArray &&a)
        {
            return a | std::views::transform([](QJsonValueRef r)
                           { return r.toString(); }) |
                std::views::transform([](QString s)
                    { return s.toLatin1(); }) |
                std::views::transform([](QByteArray ba)
                    { return QByteArray::fromBase64(ba); }) |
                std::views::transform([](QByteArray ba)
                    { return QImage::fromData(ba); }) |
                std::ranges::to<std::vector>();
        });
    return quick_launch_button;
}

quick_launch_button_t quick_launch_button_t::from_file_path(QString file_path)
{
    QFileInfo fileInfo_app(file_path);
    return quick_launch_button_t{
        .shell_execute_or_create_process{std::in_place_type<shell_execute_t>, shell_execute_t{
                                                                                  .file_path = fileInfo_app.absoluteFilePath(),
                                                                                  .window_show_state = shell_execute_t::window_show_state_name_to_code.at("SW_SHOWNORMAL"),
                                                                              }},
        .display_text_overriding = false,
        .display_text = fileInfo_app.baseName() != "" ? fileInfo_app.baseName() : fileInfo_app.absoluteFilePath(),
        .icons_overriding = false,
        .icons = {},
        .collapsed = false,
    };
}
void quick_launch_button_t::trigger(std::nullptr_t) const
{
    shell_execute_t const &shell_execute = std::get<shell_execute_t>(shell_execute_or_create_process);
    auto [lpVerb, lpFile, lpParameters, lpDirectory] = std::make_tuple(
        nullptr,
        string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(shell_execute.file_path),
        shell_execute.parameters.has_value() ? string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(shell_execute.parameters.value()) : TEXT(""),
        shell_execute.working_directory.has_value() ? string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(shell_execute.working_directory.value()) : TEXT(""));
    if(SHELLEXECUTEINFO execInfo{
           .cbSize = sizeof(execInfo),
           .fMask = SEE_MASK_INVOKEIDLIST,
           .lpVerb = NULL,
           .lpFile = lpFile.data(),
           .lpParameters = shell_execute.parameters.has_value() ? lpParameters.data() : NULL,
           .lpDirectory = shell_execute.working_directory.has_value() ? lpDirectory.data() : NULL,
           .nShow = shell_execute.window_show_state,
       };
        ShellExecuteEx(&execInfo) == FALSE)
    {
        assert(!(reinterpret_cast<int>(execInfo.hInstApp) > 32));
        ext_debug_log((shell_execute.file_path, ShellExecuteEx_error_string(reinterpret_cast<int>(execInfo.hInstApp)), GetLastErrorReturnValueToString(GetLastError(), "ShellExecuteEx")), qDebug_compact());
    }
    else
    {
        assert(reinterpret_cast<int>(execInfo.hInstApp) > 32);
    }
}
void quick_launch_button_t::trigger(QString verb) const
{
    shell_execute_t const &shell_execute = std::get<shell_execute_t>(shell_execute_or_create_process);
    auto [lpVerb, lpFile, lpParameters, lpDirectory] = std::make_tuple(
        string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(verb),
        string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(shell_execute.file_path),
        shell_execute.parameters.has_value() ? string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(shell_execute.parameters.value()) : TEXT(""),
        shell_execute.working_directory.has_value() ? string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(shell_execute.working_directory.value()) : TEXT(""));
    if(SHELLEXECUTEINFO execInfo{
           .cbSize = sizeof(execInfo),
           .fMask = SEE_MASK_INVOKEIDLIST,
           .lpVerb = lpVerb.data(),
           .lpFile = lpFile.data(),
           .lpParameters = shell_execute.parameters.has_value() ? lpParameters.data() : NULL,
           .lpDirectory = shell_execute.working_directory.has_value() ? lpDirectory.data() : NULL,
           .nShow = shell_execute.window_show_state,
       };
        ShellExecuteEx(&execInfo) == FALSE)
    {
        assert(!(reinterpret_cast<int>(execInfo.hInstApp) > 32));
        ext_debug_log((shell_execute.file_path, ShellExecuteEx_error_string(reinterpret_cast<int>(execInfo.hInstApp)), GetLastErrorReturnValueToString(GetLastError(), "ShellExecuteEx")), qDebug_compact());
    }
    else
    {
        assert(reinterpret_cast<int>(execInfo.hInstApp) > 32);
    }
}
void quick_launch_button_t::trigger() const
{
    std::visit(overloaded{[&](shell_execute_t const &shell_execute)
                   {
                       auto [lpVerb, lpFile, lpParameters, lpDirectory] = std::make_tuple(
                           shell_execute.verb.has_value() ? string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(shell_execute.verb.value()) : TEXT(""),
                           string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(shell_execute.file_path),
                           shell_execute.parameters.has_value() ? string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(shell_execute.parameters.value()) : TEXT(""),
                           shell_execute.working_directory.has_value() ? string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(shell_execute.working_directory.value()) : TEXT(""));
                       if(SHELLEXECUTEINFO execInfo{
                              .cbSize = sizeof(execInfo),
                              .fMask = SEE_MASK_INVOKEIDLIST,
                              .lpVerb = shell_execute.verb.has_value() ? lpVerb.data() : NULL,
                              .lpFile = lpFile.data(),
                              .lpParameters = shell_execute.parameters.has_value() ? lpParameters.data() : NULL,
                              .lpDirectory = shell_execute.working_directory.has_value() ? lpDirectory.data() : NULL,
                              .nShow = shell_execute.window_show_state,
                          };
                           ShellExecuteEx(&execInfo) == FALSE)
                       {
                           assert(!(reinterpret_cast<int>(execInfo.hInstApp) > 32));
                           ext_debug_log((shell_execute.file_path, ShellExecuteEx_error_string(reinterpret_cast<int>(execInfo.hInstApp)), GetLastErrorReturnValueToString(GetLastError(), "ShellExecuteEx")), qDebug_compact());
                       }
                       else
                       {
                           assert(reinterpret_cast<int>(execInfo.hInstApp) > 32);
                       }
                   },
                   [&](create_process_t const &create_process)
                   {
                       CreateProcess(NULL, // No module name (use command line)
                           string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(create_process.command_line).data(), // Command line
                           NULL, // Process handle not inheritable
                           NULL, // Thread handle not inheritable
                           FALSE, // Set handle inheritance to FALSE
                           CREATE_UNICODE_ENVIRONMENT, // No creation flags
                           create_process.environment_block.has_value() ? string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(create_process.environment_block.value()).data() : NULL, // Use parent's environment block
                           create_process.working_directory.has_value() ? string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(create_process.working_directory.value()).data() : NULL, // Use parent's starting directory
                           &unmove(STARTUPINFO{
                               .cb = sizeof(STARTUPINFO),
                               .lpTitle = create_process.title.has_value() ? string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(create_process.title.value()).data() : NULL,
                           }), // Pointer to STARTUPINFO structure
                           &unmove(PROCESS_INFORMATION{}) // Pointer to PROCESS_INFORMATION structure
                       );
                   }},
        shell_execute_or_create_process);
}


QJsonObject quick_launch_buttons_group_t::to_json_impl() const
{
    return QJsonObject({
        {"name", name},
        {"buttons", buttons | std::views::transform(&quick_launch_button_t::to_json) | std::ranges::to<QJsonArray>()},
    });
}
quick_launch_buttons_group_t quick_launch_buttons_group_t::from_json_impl(QJsonObject &obj)
{
    return quick_launch_buttons_group_t{
        .name = obj.take("name").toString(),
        .buttons = obj.take("buttons").toArray() | std::views::transform(&QJsonValueRef::toObject) | std::views::transform(&quick_launch_button_t::from_json) |
            std::ranges::to<std::vector>(),
    };
}

QJsonObject quick_launch_t::to_json_impl() const
{
    return QJsonObject({
        {"directories", directories | std::ranges::to<QJsonArray>()},
        {"buttons_groups", buttons_groups | std::views::transform(&quick_launch_buttons_group_t::to_json) | std::ranges::to<QJsonArray>()},
    });
}
quick_launch_t quick_launch_t::from_json_impl(QJsonObject &obj)
{
    return quick_launch_t{
        .directories = obj.take("directories").toArray() | std::views::transform([](QJsonValueRef v)
                                                               { return v.toString(); }) |
            std::ranges::to<std::vector>(),
        .buttons_groups = obj.take("buttons_groups").toArray() | std::views::transform(&QJsonValueRef::toObject) | std::views::transform(&quick_launch_buttons_group_t::from_json) |
            std::ranges::to<std::vector>(),
    };
}

quick_launch_t quick_launch_t::from_config_file_and_file_system(QString file_path)
{
    quick_launch_t quick_launch;
    if(QFile quick_launch_settings_file(file_path); quick_launch_settings_file.open(QFile::OpenModeFlag::ReadOnly))
    {
        quick_launch = quick_launch_t::from_json(QJsonDocument::fromJson(quick_launch_settings_file.readAll()).object());
    }

    std::map<std::basic_string<TCHAR>, std::basic_string<TCHAR>> env;
    get_envoronment_variables(std::inserter(env, env.end()));
    //    ext_debug_log((env), qDebug_compact());
    if(auto [contains_APPDATA, it_APPDATA] = contains_and_find(env, TEXT("APPDATA")); contains_APPDATA)
    {
        push_back_if_did_not_find(quick_launch.directories, string_or_string_view_to_string_or_string_view<QString>(it_APPDATA->second + TEXT(R"(\Microsoft\Internet Explorer\Quick Launch)")));
    }

    for(QFileInfo fileInfo_app : std::views::concat(
            quick_launch.directories | std::views::transform([](QString quick_launch_directory)
                                           { return std::ranges::subrange(ext_dir_iterator(quick_launch_directory, QStringList(), QDir::Filter::NoDot | QDir::Filter::NoDotDot | QDir::Filter::AllEntries | QDir::Filter::Hidden | QDir::Filter::System, QDirIterator::IteratorFlag::Subdirectories), std::default_sentinel); }) |
                std::views::join | std::ranges::to<std::vector>() // std::views::join https://stackoverflow.com/questions/69788336/using-stdviewsfilter-after-stdviewsjoin-does-not-compile
            ,
            std::array{QDir::homePath(), QDir::rootPath(), QDir::tempPath()} | std::views::transform(boost::value_factory<QFileInfo>()),
            QDir::drives()))
    {
        push_back_if_did_not_find_if_nested(
            quick_launch.buttons_groups,
            std::mem_fn(&quick_launch_buttons_group_t::buttons),
            [&](quick_launch_button_t &quick_launch_button)
            { return std::visit(overloaded{[&](quick_launch_button_t::shell_execute_t &shell_execute)
                                    {
                                        return shell_execute.file_path == fileInfo_app.absoluteFilePath();
                                    },
                                    [&](quick_launch_button_t::create_process_t & /*create_process*/)
                                    {
                                        return false;
                                    }},
                  quick_launch_button.shell_execute_or_create_process); },
            [&](quick_launch_buttons_group_t &quick_launch_buttons_group)
            { return quick_launch_buttons_group.name == "default"; },
            []()
            { return quick_launch_buttons_group_t{.name = "default", .buttons = {}}; },
            [&]
            { return quick_launch_button_t::from_file_path(fileInfo_app.absoluteFilePath()); });
    }
    return quick_launch;
}
void quick_launch_t::to_config_file(QString file_path)
{
    if(QSaveFile quick_launch_settings_file(file_path); quick_launch_settings_file.open(QFile::OpenModeFlag::WriteOnly))
    {
        quick_launch_settings_file.write(QJsonDocument(to_json()).toJson());
        quick_launch_settings_file.commit();
    }
}

void quick_launch_t::refresh_config_file(QWidget *widget)
{
    to_config_file(widget->property("config_file_file_path").toString());
}
void quick_launch_t::refresh_text_edit(QWidget *widget)
{
    qvariant_cast<QPlainTextEdit *>(widget->property("text_edit"))->setPlainText(QJsonDocument(to_json()).toJson());
}

void quick_launch_button_t::refresh_window_menu(QMenu *window_menu)
{
    std::visit(overloaded{[&](shell_execute_t &shell_execute)
                   {
                       qvariant_cast<QMenu *>(window_menu->property("file_permissions_window_menu"))->menuAction()->setVisible(true);
                       qvariant_cast<QMenu *>(window_menu->property("choose_verbs_window_menu"))->menuAction()->setVisible(true);
                       QMap<QString, QVariant> file_permission_menus = window_menu->property("file_permission_menus").toMap();
                       if(WIN32_FILE_ATTRIBUTE_DATA attr; GetFileAttributesEx(string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(shell_execute.file_path).data(), GetFileExInfoStandard, &attr) == 0)
                       {
                           qvariant_cast<QPlainTextEdit *>(window_menu->property("file_permissions_error_string_text_edit"))->appendPlainText(QDateTime::currentDateTime().toString(Qt::DateFormat::ISODateWithMs) + " " + string_or_string_view_to_string_or_string_view<QString>(GetLastErrorReturnValueToString(GetLastError(), "GetFileAttributesEx")));
                       }
                       else
                       {
                           qvariant_cast<QPlainTextEdit *>(window_menu->property("file_permissions_error_string_text_edit"))->appendPlainText(QDateTime::currentDateTime().toString(Qt::DateFormat::ISODateWithMs) + " " + "GetFileAttributesEx succeeded");
                           for(auto [name, p] : FILE_ATTRIBUTE_label_to_value_mapping)
                           {
                               qvariant_cast<QAction *>(file_permission_menus.value(name))->blockSignals(true);
                               qvariant_cast<QAction *>(file_permission_menus.value(name))->setChecked(attr.dwFileAttributes & p);
                               qvariant_cast<QAction *>(file_permission_menus.value(name))->blockSignals(false);
                           }
                       }
                   },
                   [&](create_process_t & /*create_process*/)
                   {
                       qvariant_cast<QMenu *>(window_menu->property("file_permissions_window_menu"))->menuAction()->setVisible(false);
                       qvariant_cast<QMenu *>(window_menu->property("choose_verbs_window_menu"))->menuAction()->setVisible(false);
                   }},
        shell_execute_or_create_process);
}

void quick_launch_button_t::refresh_button(QToolButton *button)
{
    button->setText(escape_ampersand(display_text));
    if(icons_overriding)
    {
        button->setIcon(QImages_to_QIcon(icons.begin(), icons.end()));
    }
    else
    {
        button->setIcon(placeholder_icon());
        std::visit(overloaded{[&](quick_launch_button_t::shell_execute_t &shell_execute)
                       {
                           [](auto file_path, auto pbutton) -> future_t<void>
                           {
                               if(auto optional_icon = filePath_to_QIcon_gui(file_path).or_else([&]
                                      { return filePath_to_QIcon_widgets(file_path); });
                                   optional_icon.has_value())
                               {
                                   QIcon icon = *optional_icon;
                                   icon = file_is_hidden(file_path) ? co_await opacity_icon(icon) : icon;
                                   if(auto qpbutton = *pbutton; !qpbutton.isNull())
                                   {
                                       qpbutton->setIcon(icon);
                                       //                                            set_widget_opacity(button, file_is_hidden(shell_execute.file_path) ? 0.5 : 1.0);
                                   }
                               }
                           }(shell_execute.file_path, std::make_unique<QPointer<QToolButton>>(button));
                       },
                       [&](quick_launch_button_t::create_process_t &create_process)
                       {
                           [](auto command_line, auto pbutton) -> future_t<void>
                           {
                               co_await coroutine_dispatch_to_thread_t{retrieve_icon_thread()};

                               std::vector<std::wstring> command_args;
                               split_command(string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(command_line), std::back_inserter(command_args));

                               for(QString subarg : command_args | std::views::transform([](std::wstring const &command_arg)
                                                                       { return string_or_string_view_to_string_or_string_view<QString>(command_arg).split(","); }) |
                                       std::views::join | std::ranges::to<std::vector>() | std::views::reverse)
                               {
                                   auto [filePathsStr, _] = where_result(subarg);
                                   //                             ext_debug_log((where_result(subarg)), qDebug_compact());
                                   auto filePathsVec = filePathsStr.split("\r\n") | std::views::filter([](auto &filePath)
                                                                                        { return !filePath.isEmpty() && !filePath.contains("shell32") && !filePath.contains("rundll32"); }) |
                                       std::ranges::to<std::vector>();
                                   for(auto filePath : filePathsVec)
                                   {
                                       std::vector<std::vector<QImage>> icons_groups;
                                       //                                 ext_debug_log((filePath), qDebug_compact());
                                       get_images_EnumResourceNames<"icon", true>(string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(filePath).data(), std::back_inserter(icons_groups), [&](auto &&)
                                           { return std::back_inserter(icons_groups.back()); });
                                       //                                   ext_debug_log((), qDebug_compact());
                                       if(!icons_groups.empty())
                                       {
                                           //                                     ext_debug_log((icons_groups.size()), qDebug_compact());
                                           co_await coroutine_dispatch_to_main_thread_t{};
                                           if(auto qpbutton = *pbutton; !qpbutton.isNull())
                                           {
                                               qpbutton->setIcon(QImages_to_QIcon(icons_groups[0].begin(), icons_groups[0].end()));
                                           }
                                           goto stop_looking_for_icon;
                                       }
                                   }
                               stop_looking_for_icon:;
                               }
                           }(create_process.command_line, std::make_unique<QPointer<QToolButton>>(button));
                       }},
            shell_execute_or_create_process);
    }
}
Q_DECLARE_METATYPE(quick_launch_button_t)
void quick_launch_t::refresh_widget_inner(QWidget *root_widget)
{
    QHash<quick_launch_button_t *, QVariant> buttons;

    GroupBox *group_collapsed = new GroupBox(u"collapsed"_s_esc_amp);
    FlowLayout *group_collapsed_flowlayout = new FlowLayout(group_collapsed);

    QWidget *quick_launch_groups_widget_inner = new QWidget();
    QVBoxLayout *quick_launch_groups_vlayout_inner = new QVBoxLayout(quick_launch_groups_widget_inner);
    for(quick_launch_buttons_group_t &quick_launch_buttons_group : buttons_groups)
    {
        GroupBox *group = new GroupBox(escape_ampersand(quick_launch_buttons_group.name));
        FlowLayout *group_flowlayout = new FlowLayout(group);
        for(quick_launch_button_t &quick_launch_button : quick_launch_buttons_group.buttons)
        {
            QToolButton *button = new QToolButton();
            button->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
            button->setIconSize(QSize(32, 32));
            buttons.insert(&quick_launch_button, QVariant::fromValue<QObject *>(button));
            QObject::connect(button, &QToolButton::clicked, button, [&quick_launch_button]()
                { quick_launch_button.trigger(); });
            (quick_launch_button.collapsed ? group_collapsed_flowlayout : group_flowlayout)->addWidget(button);

            button->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
            auto get_button = [root_widget, &quick_launch_button]
            {
                decltype(buttons)buttons= qvariant_cast<decltype(buttons)>(root_widget->property("buttons"));
                decltype(button)button=qvariant_cast<decltype(button)>(buttons.value(&quick_launch_button));
                return button; };
            QObject::connect(button, &QWidget::customContextMenuRequested, button, [this, root_widget, &quick_launch_button, get_button](QPoint const &pos)
                {
                    decltype(button)button=get_button();
                  QMenu*window_menu=new QMenu(root_widget);
                  window_menu->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
//                  QObject::connect(window_menu, &QMenu::destroyed, window_menu, []() { ext_debug_log((), qDebug_compact()); });
                  //                  make_menu_top_right_menu(window_menu,button->mapToGlobal(pos));
                  make_menu_in_screen(window_menu);
                  attach_widget_consuming_click(window_menu);
                  QVBoxLayout*menu_widget_consuming_click_vlayout=new QVBoxLayout(qvariant_cast<QWidget *>(window_menu->property("widget_consuming_click")));

                  QHBoxLayout *file_icon_and_file_name_hlayout = new QHBoxLayout();
                  {
                      for(QSize size : button->icon().availableSizes())
                      {
                          QPixmap pixmap = button->icon().pixmap(size);
                          QPushButton *icon_button = new QPushButton();
                          icon_button->setIconSize(size);
                          QIcon button_icon;
                          button_icon.addPixmap(pixmap);
                          button_icon.addPixmap(pixmap, QIcon::Mode::Disabled);
                          icon_button->setIcon(button_icon);
                          //                        ext_debug_log((size, pixmap.size(), icon_button->iconSize(), button_icon.availableSizes()), qDebug_compact());
                          QObject::connect(icon_button, &QPushButton::clicked, icon_button, [pixmap]()
                              {QMimeData*mimeData=new QMimeData();mimeData->setImageData(pixmap);QGuiApplication::clipboard()->setMimeData(mimeData); });
                          file_icon_and_file_name_hlayout->addWidget(icon_button);
                      }
                      file_icon_and_file_name_hlayout->addStretch(1);
                  }
                  menu_widget_consuming_click_vlayout->addLayout(file_icon_and_file_name_hlayout);

                  QMenu *menu_menu = new QMenu();
                  make_top_level_menu_not_hide_when_toggling_checkable_child_and_not_hide_and_close_some_window_menu_when_clicking_child(menu_menu, [window_menu]
                    { window_menu->close(); });
                  make_top_level_menu_not_shrink(menu_menu);

                  QAction*collapsion_toggle_action=new QAction("collapsed",menu_menu);
                  collapsion_toggle_action->setCheckable(true);
                  collapsion_toggle_action->setChecked(quick_launch_button.collapsed);
                  QObject::connect(collapsion_toggle_action,&QAction::toggled,collapsion_toggle_action,[this,root_widget,&quick_launch_button](bool checked){
                      quick_launch_button.collapsed=checked;
                      this->refresh_config_file(root_widget);
                      this->refresh_widget_inner(root_widget);
                      this->refresh_text_edit(root_widget);
                  });
                  menu_menu->addAction(collapsion_toggle_action);

                  QMenu *file_permissions_menu=new QMenu();
                  {
                      make_non_top_level_menu_not_hide_when_toggling_checkable_child(file_permissions_menu);
                      make_top_level_menu_not_shrink(file_permissions_menu);
                      QMap<QString,QVariant>file_permission_menus;
                      for(auto[name,p]:FILE_ATTRIBUTE_label_to_value_mapping)
                      {
                          QAction*file_permission_toggle_action=new QAction(name,file_permissions_menu);
                          file_permission_toggle_action->setCheckable(true);
                          auto get_file_path=[&quick_launch_button](){return std::get<quick_launch_button_t::shell_execute_t>(quick_launch_button.shell_execute_or_create_process).file_path;};
                          auto on_GetFileAttributesEx_failed = [window_menu](QString error_string)
                          { qvariant_cast<QPlainTextEdit *>(window_menu->property("file_permissions_error_string_text_edit"))->appendPlainText(QDateTime::currentDateTime().toString(Qt::DateFormat::ISODateWithMs) + " " + error_string); };
                          auto on_SetFileAttributes_failed=[window_menu](QString error_string){qvariant_cast<QPlainTextEdit *>(window_menu->property("file_permissions_error_string_text_edit"))->appendPlainText(QDateTime::currentDateTime().toString(Qt::DateFormat::ISODateWithMs)+" "+error_string);};
                          auto on_succeeded = [&quick_launch_button, get_button, window_menu]()
                          {
                              decltype(button)button=get_button();
                              quick_launch_button.refresh_button(button);qvariant_cast<QPlainTextEdit *>(window_menu->property("file_permissions_error_string_text_edit"))->appendPlainText(QDateTime::currentDateTime().toString(Qt::DateFormat::ISODateWithMs)+" "+"GetFileAttributesEx and SetFileAttributes succeeded"); };
                          QObject::connect(file_permission_toggle_action,&QAction::toggled,file_permission_toggle_action,
                            [get_file_path,on_GetFileAttributesEx_failed,on_SetFileAttributes_failed,on_succeeded,p](bool checked){
                              if(WIN32_FILE_ATTRIBUTE_DATA attr;GetFileAttributesEx(string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(get_file_path()).data(),GetFileExInfoStandard,&attr)==0)
                              {
                                  on_GetFileAttributesEx_failed(string_or_string_view_to_string_or_string_view<QString>(GetLastErrorReturnValueToString(GetLastError(), "GetFileAttributesEx")));
                              }
                              else
                              {
                                  if(SetFileAttributes(string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(get_file_path()).data(),(attr.dwFileAttributes&~p)|(checked?p:0))==0)
                                  {
                                      on_SetFileAttributes_failed(string_or_string_view_to_string_or_string_view<QString>(GetLastErrorReturnValueToString(GetLastError(), "SetFileAttributes")));
                                  }
                                  else
                                  {
                                      on_succeeded();
                                  }
                              }
                          });
                          file_permissions_menu->addAction(file_permission_toggle_action);
                          file_permission_menus.insert(name,QVariant::fromValue<QObject *>(file_permission_toggle_action));
                      }
                      window_menu->setProperty("file_permission_menus", file_permission_menus);
                  }

                  QMenu *file_permissions_window_menu=new QMenu(u"file_permissions"_s_esc_amp,menu_menu);
                  {
                      make_menu_in_screen(file_permissions_window_menu);
                      window_menu->setProperty("file_permissions_window_menu", QVariant::fromValue<QObject *>(file_permissions_window_menu));
                      attach_widget_consuming_click(file_permissions_window_menu);
                      QVBoxLayout*file_permissions_window_menu_widget_consuming_click_vlayout=new QVBoxLayout(qvariant_cast<QWidget *>(file_permissions_window_menu->property("widget_consuming_click")));
                      file_permissions_window_menu_widget_consuming_click_vlayout->addWidget(file_permissions_menu);
                      QPlainTextEdit*file_permissions_error_string_text_edit=new QPlainTextEdit();
                      make_plain_text_edit_readonly(file_permissions_error_string_text_edit);
                      file_permissions_error_string_text_edit->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
                      window_menu->setProperty("file_permissions_error_string_text_edit", QVariant::fromValue<QObject *>(file_permissions_error_string_text_edit));
                      file_permissions_window_menu_widget_consuming_click_vlayout->addWidget(file_permissions_error_string_text_edit);
                      QPushButton*file_permissions_refresh_button=new QPushButton(u"refresh"_s_esc_amp);
                      auto refresh=[&quick_launch_button,window_menu](){quick_launch_button.refresh_window_menu(window_menu);};
                      QObject::connect(file_permissions_refresh_button,&QPushButton::clicked,file_permissions_refresh_button,[refresh](){
                          refresh();
                      });
                      file_permissions_window_menu_widget_consuming_click_vlayout->addWidget(file_permissions_refresh_button);
                  }
                  menu_menu->addMenu(file_permissions_window_menu);

                  auto trigger_verb=overloaded{
                      [&quick_launch_button,window_menu](QString verb){
                          quick_launch_button.trigger(verb);
                              window_menu->close();
                      },[&quick_launch_button,window_menu](std::nullptr_t){
                          quick_launch_button.trigger(nullptr);
                              window_menu->close();
                      },};
                  QMenu *choose_verbs_menu=new QMenu();
                  {
                      make_top_level_menu_not_shrink(choose_verbs_menu);
                      for(QString verb:{"edit","explore","find","open","print","properties","runas",})
                      {
                          QAction*choose_verb_action=new QAction(verb,choose_verbs_menu);
                          QObject::connect(choose_verb_action,&QAction::triggered,choose_verb_action,[trigger_verb,verb](){
                              trigger_verb(verb);
                          });
                          choose_verbs_menu->addAction(choose_verb_action);
                      }
                      {
                          QAction*choose_verb_action=new QAction("(NULL)",choose_verbs_menu);
                          QObject::connect(choose_verb_action,&QAction::triggered,choose_verb_action,[trigger_verb](){
                              trigger_verb(nullptr);
                          });
                          choose_verbs_menu->addAction(choose_verb_action);
                      }
                  }

                  QMenu *choose_verbs_window_menu=new QMenu(u"choose_verbs"_s_esc_amp,menu_menu);
                  {
                      make_menu_in_screen(choose_verbs_window_menu);
                      window_menu->setProperty("choose_verbs_window_menu", QVariant::fromValue<QObject *>(choose_verbs_window_menu));
                      attach_widget_consuming_click(choose_verbs_window_menu);
                      QVBoxLayout*choose_verbs_window_menu_widget_consuming_click_vlayout=new QVBoxLayout(qvariant_cast<QWidget *>(choose_verbs_window_menu->property("widget_consuming_click")));
                      choose_verbs_window_menu_widget_consuming_click_vlayout->addWidget(choose_verbs_menu);
                      QPlainTextEdit*choose_verbs_text_edit=new QPlainTextEdit();
                      choose_verbs_text_edit->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
                      QPushButton*choose_verbs_button=new QPushButton(u"trigger"_s_esc_amp);
                      QObject::connect(choose_verbs_button,&QPushButton::clicked,choose_verbs_button,[trigger_verb,choose_verbs_text_edit](){
                          trigger_verb(choose_verbs_text_edit->toPlainText());
                      });
                      QHBoxLayout*choose_verbs_custom_hlayout=new QHBoxLayout();
                      choose_verbs_window_menu_widget_consuming_click_vlayout->addWidget(choose_verbs_text_edit);
                      choose_verbs_window_menu_widget_consuming_click_vlayout->addWidget(choose_verbs_button);
                      choose_verbs_window_menu_widget_consuming_click_vlayout->addLayout(choose_verbs_custom_hlayout);
                  }
                  menu_menu->addMenu(choose_verbs_window_menu);

                  QHBoxLayout *menu_menu_hlayout = new QHBoxLayout();
                  menu_menu_hlayout->addWidget(menu_menu);
                  menu_menu_hlayout->addStretch(1);
                  menu_widget_consuming_click_vlayout->addLayout(menu_menu_hlayout);

                  QPlainTextEdit*quick_launch_button_text_edit=new QPlainTextEdit(QJsonDocument(quick_launch_button.to_json()).toJson());
                  quick_launch_button_text_edit->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
                  menu_widget_consuming_click_vlayout->addWidget(quick_launch_button_text_edit);
                  QPushButton*quick_launch_button_save=new QPushButton(u"save"_s_esc_amp);
                  QObject::connect(quick_launch_button_save,&QPushButton::clicked,quick_launch_button_save,[this,root_widget,&quick_launch_button, get_button,window_menu,quick_launch_button_save,quick_launch_button_text_edit](){
                      decltype(button)button=get_button();
                      QJsonParseError e;
                      QByteArray text_utf8=quick_launch_button_text_edit->toPlainText().toUtf8();
                      QJsonDocument d=QJsonDocument::fromJson(text_utf8,&e);
                      if(d.isNull())
                      {
                          quick_launch_button_save->setText(u"save\n"_s_esc_amp+escape_ampersand(e.errorString()));
                          QTextCursor c=quick_launch_button_text_edit->textCursor();
                          c.setPosition(QString::fromUtf8(QByteArrayView(text_utf8).first(e.offset)).size());
                          quick_launch_button_text_edit->setTextCursor(c);
                          quick_launch_button_text_edit->setFocus();
                      }
                      else
                      {
                          quick_launch_button_save->setText(u"save"_s_esc_amp);
                          quick_launch_button = quick_launch_button_t::from_json(d.object());
                          this->refresh_config_file(root_widget);
                          this->refresh_text_edit(root_widget);
                          quick_launch_button.refresh_button(button);
                          quick_launch_button.refresh_window_menu(window_menu);
                      }
                  });
                  QPushButton*quick_launch_button_reset=new QPushButton(u"reset"_s_esc_amp);
                  QObject::connect(quick_launch_button_reset,&QPushButton::clicked,quick_launch_button_reset,[&quick_launch_button,quick_launch_button_text_edit](){
                      quick_launch_button_text_edit->setPlainText(QJsonDocument(quick_launch_button.to_json()).toJson());
                  });
                  QHBoxLayout*quick_launch_button_buttons_hlayout=new QHBoxLayout();
                  quick_launch_button_buttons_hlayout->addWidget(quick_launch_button_save);
                  quick_launch_button_buttons_hlayout->addWidget(quick_launch_button_reset);
                  menu_widget_consuming_click_vlayout->addLayout(quick_launch_button_buttons_hlayout);

                  QRect screenGeometry = root_widget->screen()->geometry();
                  quick_launch_button_text_edit->setMinimumHeight(screenGeometry.height() * 0.22);
                  window_menu->setMinimumWidth(screenGeometry.width() * 0.35);

                  quick_launch_button.refresh_window_menu(window_menu);
                  window_menu->popup(button->mapToGlobal(pos)); });

            quick_launch_button.refresh_button(button);
        }
        quick_launch_groups_vlayout_inner->addWidget(group);
    }
    root_widget->setProperty("buttons", QVariant::fromValue(buttons));
    quick_launch_groups_vlayout_inner->addWidget(group_collapsed);

    QWidget *quick_launch_groups_widget_inner_old = qvariant_cast<QWidget *>(root_widget->property("widget_inner"));
    delete quick_launch_groups_widget_inner_old->parentWidget()->layout()->replaceWidget(quick_launch_groups_widget_inner_old, quick_launch_groups_widget_inner);
    quick_launch_groups_widget_inner_old->deleteLater();
    root_widget->setProperty("widget_inner", QVariant::fromValue<QObject *>(quick_launch_groups_widget_inner));
}

QWidget *quick_launch_t::create_widget(QString config_file_file_path)
{
    QTabWidget *quick_launch_toggle_show_or_edit = new QTabWidget();
    quick_launch_toggle_show_or_edit->setProperty("config_file_file_path", config_file_file_path);

    QWidget *quick_launch_groups_widget_inner = new QWidget();
    quick_launch_toggle_show_or_edit->setProperty("widget_inner", QVariant::fromValue<QObject *>(quick_launch_groups_widget_inner));

    QWidget *quick_launch_groups_widget_outer = new QWidget();
    QVBoxLayout *quick_launch_groups_vlayout_outer = new QVBoxLayout(quick_launch_groups_widget_outer);
    quick_launch_groups_vlayout_outer->setContentsMargins(QMargins());
    quick_launch_groups_vlayout_outer->addWidget(quick_launch_groups_widget_inner);
    quick_launch_groups_vlayout_outer->addStretch(1);
    refresh_widget_inner(quick_launch_toggle_show_or_edit);
    ScrollAreaVertical *quick_launch_groups_scroll_area = new ScrollAreaVertical(quick_launch_groups_widget_outer);

    QPlainTextEdit *quick_launch_config_text_edit = new QPlainTextEdit(QJsonDocument(to_json()).toJson());
    quick_launch_config_text_edit->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
    quick_launch_toggle_show_or_edit->setProperty("text_edit", QVariant::fromValue<QObject *>(quick_launch_config_text_edit));
    QPushButton *quick_launch_config_save = new QPushButton(u"save && merge with file system state"_s_esc_amp);
    QObject::connect(quick_launch_config_save, &QPushButton::clicked, quick_launch_config_save, [this, quick_launch_config_save, quick_launch_config_text_edit, quick_launch_toggle_show_or_edit]()
        {
            QJsonParseError e;
          QByteArray text_utf8=quick_launch_config_text_edit->toPlainText().toUtf8();
            QJsonDocument d=QJsonDocument::fromJson(text_utf8,&e);
            if(d.isNull())
            {
                quick_launch_config_save->setText(u"save && merge with file system state\n"_s_esc_amp+escape_ampersand(e.errorString()));
                QTextCursor c=quick_launch_config_text_edit->textCursor();
                c.setPosition(QString::fromUtf8(QByteArrayView(text_utf8).first(e.offset)).size());
                quick_launch_config_text_edit->setTextCursor(c);
                quick_launch_config_text_edit->setFocus();
            }
            else
            {
                quick_launch_config_save->setText(u"save && merge with file system state"_s_esc_amp);
                *this = quick_launch_t::from_json(d.object());
                refresh_config_file(quick_launch_toggle_show_or_edit);
                *this = from_config_file_and_file_system(quick_launch_toggle_show_or_edit->property("config_file_file_path").toString());
                refresh_widget_inner(quick_launch_toggle_show_or_edit);
                refresh_text_edit(quick_launch_toggle_show_or_edit);
                quick_launch_toggle_show_or_edit->setCurrentIndex(0);
            } });
    QPushButton *quick_launch_config_cancel = new QPushButton(u"reset"_s_esc_amp);
    QObject::connect(quick_launch_config_cancel, &QPushButton::clicked, quick_launch_config_cancel, [this, quick_launch_toggle_show_or_edit]()
        { refresh_text_edit(quick_launch_toggle_show_or_edit); });
    QHBoxLayout *quick_launch_config_buttons_hlayout = new QHBoxLayout();
    quick_launch_config_buttons_hlayout->addWidget(quick_launch_config_save);
    quick_launch_config_buttons_hlayout->addWidget(quick_launch_config_cancel);

    QWidget *quick_launch_config = new QWidget();
    QVBoxLayout *quick_launch_config_vlayout = new QVBoxLayout(quick_launch_config);
    quick_launch_config_vlayout->addWidget(quick_launch_config_text_edit);
    quick_launch_config_vlayout->addLayout(quick_launch_config_buttons_hlayout);

    quick_launch_toggle_show_or_edit->addTab(quick_launch_groups_scroll_area, "use");
    quick_launch_toggle_show_or_edit->addTab(quick_launch_config, "config");

    return quick_launch_toggle_show_or_edit;
}
