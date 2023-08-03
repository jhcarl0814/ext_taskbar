#include "start_menu.hpp"

#include "ext_infrastructure/ext_debug.hpp"
#include "ext_infrastructure/ext_string.hpp"
#include "ext_infrastructure/ext_error_handling.hpp"
#include "ext_infrastructure/ext_thread.hpp"
#include "ext_infrastructure/ext_configs.hpp"
#include "ext_infrastructure/ext_algorithm.hpp"

#include <QFileInfo>
#include <QMimeData>
#include "ext_core/ext_file_info.hpp"

#include <QClipboard>
#include "ext_gui/ext_image.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include "ext_widgets/escape_ampersand.hpp"
#include "ext_widgets/menu_permanent.hpp"
#include "ext_widgets/menu_nested.hpp"
#include "ext_widgets/menu_window.hpp"
#include "ext_widgets/menu_positioned.hpp"
#include "ext_widgets/ext_text_edit.hpp"
#include "ext_widgets/ext_groupbox.hpp"
#include "ext_widgets/scroll_area_vertical.hpp"

void trigger_action(QAction *a, std::nullptr_t)
{
    auto [lpVerb, lpFile, lpParameters, lpDirectory] = std::make_tuple(
        nullptr,
        string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(a->property("absoluteFilePath").toString()),
        nullptr,
        nullptr);
    if(SHELLEXECUTEINFO execInfo{
           .cbSize = sizeof(execInfo),
           .fMask = SEE_MASK_INVOKEIDLIST,
           .lpVerb = lpVerb,
           .lpFile = lpFile.data(),
           .lpParameters = lpParameters,
           .lpDirectory = lpDirectory,
           .nShow = SW_SHOWDEFAULT,
       };
        ShellExecuteEx(&execInfo) == FALSE)
    {
        assert(!(reinterpret_cast<int>(execInfo.hInstApp) > 32));
        ext_debug_log((a->property("absoluteFilePath").toString(), ShellExecuteEx_error_string(reinterpret_cast<int>(execInfo.hInstApp)), GetLastErrorReturnValueToString(GetLastError(), "ShellExecuteEx")), qDebug_compact());
    }
    else
    {
        assert(reinterpret_cast<int>(execInfo.hInstApp) > 32);
    }
}

void trigger_action(QAction *a, QString verb)
{
    auto [lpVerb, lpFile, lpParameters, lpDirectory] = std::make_tuple(
        string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(verb),
        string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(a->property("absoluteFilePath").toString()),
        nullptr,
        nullptr);
    if(SHELLEXECUTEINFO execInfo{
           .cbSize = sizeof(execInfo),
           .fMask = SEE_MASK_INVOKEIDLIST,
           .lpVerb = lpVerb.data(),
           .lpFile = lpFile.data(),
           .lpParameters = lpParameters,
           .lpDirectory = lpDirectory,
           .nShow = SW_SHOWDEFAULT,
       };
        ShellExecuteEx(&execInfo) == FALSE)
    {
        assert(!(reinterpret_cast<int>(execInfo.hInstApp) > 32));
        ext_debug_log((a->property("absoluteFilePath").toString(), verb, ShellExecuteEx_error_string(reinterpret_cast<int>(execInfo.hInstApp)), GetLastErrorReturnValueToString(GetLastError(), "ShellExecuteEx")), qDebug_compact());
    }
    else
    {
        assert(reinterpret_cast<int>(execInfo.hInstApp) > 32);
    }
}

void refresh_action(QAction *a)
{
    QFileInfo file_file_Info(a->property("absoluteFilePath").toString());
    a->setText(escape_ampersand(file_file_Info.fileName()));

    [](auto absoluteFilePath, auto paction) -> future_t<void>
    {
        if(auto optional_icon = filePath_to_QIcon_gui(absoluteFilePath).or_else([&]
               { return filePath_to_QIcon_widgets(absoluteFilePath); });
            optional_icon.has_value())
        {
            QIcon icon = *optional_icon;
            icon = file_is_hidden(absoluteFilePath) ? co_await opacity_icon(icon) : icon;
            if(auto qpaction = *paction; !qpaction.isNull())
                qpaction->setIcon(icon);
        }
    }(file_file_Info.absoluteFilePath(), std::make_unique<QPointer<QAction>>(a));
    //    a->setIcon((file_is_hidden(file_file_Info.absoluteFilePath()) ? opacity_icon : +[](QIcon icon)
    //                                                                                   { return icon; })(filePath_to_QIcon(file_file_Info.absoluteFilePath())));
}

void refresh_window_menu(QAction *a, QMenu *window_menu)
{
    QMap<QString, QVariant> file_permission_menus = window_menu->property("file_permission_menus").toMap();
    if(WIN32_FILE_ATTRIBUTE_DATA attr; GetFileAttributesEx(string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(a->property("absoluteFilePath").toString()).data(), GetFileExInfoStandard, &attr) == 0)
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
}

void MouseRightButtonPress_handler(QMenu *m, QAction *a_in_m, QPoint pos_in_m, QMenu * /*event_receiver*/)
{
    //    ext_debug_log((a, m, pos, a->property("absoluteFilePath").toString()), qDebug_compact());

    if(!a_in_m->property("absoluteFilePath").isValid())
    {
        assert(a_in_m->menu() == nullptr);
        assert(a_in_m->text() == "(empty)");
    }
    else
    {
        QWidget *parentWidget;
        bool isDir;
        if(QFileInfo info(a_in_m->property("absoluteFilePath").toString()); info.isDir())
        {
            assert(a_in_m->menu() != nullptr);
            parentWidget = a_in_m->menu();
            isDir = true;
        }
        else
        {
            assert(a_in_m->menu() == nullptr);
            parentWidget = m;
            isDir = false;
        }

        QMenu *window_menu = new QMenu(parentWidget);
        QObject::connect(window_menu, &QMenu::aboutToShow, window_menu, [a_in_m /*, event_receiver*/]()
            {
                a_in_m->setEnabled(false);
                if(a_in_m->menu() != nullptr)
                {
                    a_in_m->menu()->close();
                }
                //                if(a_in_m->menu()!=event_receiver)
                //                {
                //                    event_receiver->close();
                //                }
            });
        QObject::connect(window_menu, &QMenu::aboutToHide, window_menu, [a_in_m]()
            { a_in_m->setEnabled(true); });
        window_menu->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
        make_menu_in_screen(window_menu);
        attach_widget_consuming_click(window_menu);
        QVBoxLayout *menu_widget_consuming_click_vlayout = new QVBoxLayout(qvariant_cast<QWidget *>(window_menu->property("widget_consuming_click")));

        QHBoxLayout *file_icon_and_file_name_hlayout = new QHBoxLayout();
        {
            QFileInfo file_file_Info(a_in_m->property("absoluteFilePath").toString());
            [](auto absoluteFilePath, auto playout) -> future_t<void>
            {
                if(auto optional_icon = filePath_to_QIcon_gui(absoluteFilePath).or_else([&]
                       { return filePath_to_QIcon_widgets(absoluteFilePath); });
                    optional_icon.has_value())
                {
                    QIcon icon = *optional_icon;
                    icon = file_is_hidden(absoluteFilePath) ? co_await opacity_icon(icon) : icon;
                    int i = 0;
                    for(QSize size : icon.availableSizes())
                    {
                        QPixmap pixmap = icon.pixmap(size);
                        QPushButton *icon_button = new QPushButton();
                        icon_button->setIconSize(size);
                        QIcon button_icon;
                        button_icon.addPixmap(pixmap);
                        button_icon.addPixmap(pixmap, QIcon::Mode::Disabled);
                        icon_button->setIcon(button_icon);
                        //                        ext_debug_log((size, pixmap.size(), icon_button->iconSize(), button_icon.availableSizes()), qDebug_compact());
                        QObject::connect(icon_button, &QPushButton::clicked, icon_button, [pixmap]()
                            {QMimeData*mimeData=new QMimeData();mimeData->setImageData(pixmap);QGuiApplication::clipboard()->setMimeData(mimeData); });
                        if(auto qplayout = *playout; !qplayout.isNull())
                        {
                            qplayout->insertWidget(i, icon_button);
                            ++i;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }(file_file_Info.absoluteFilePath(), std::make_unique<QPointer<QHBoxLayout>>(file_icon_and_file_name_hlayout));
            file_icon_and_file_name_hlayout->addWidget(new QLabel(escape_ampersand(file_file_Info.fileName())));
            file_icon_and_file_name_hlayout->addStretch(1);
        }
        menu_widget_consuming_click_vlayout->addLayout(file_icon_and_file_name_hlayout);

        QFormLayout *file_paths_formlayout = new QFormLayout();
        {
            QFileInfo fileInfo(a_in_m->property("absoluteFilePath").toString());

            auto add = [&](QString method_name, QString (QFileInfo::*method)() const)
            {
                QPushButton *copy_file_path_button = new QPushButton(escape_ampersand(method_name));
                QObject::connect(copy_file_path_button, &QPushButton::clicked, copy_file_path_button, [s = (fileInfo.*method)()]()
                    { QGuiApplication::clipboard()->setText(s); });
                QLineEdit *file_path_text_edit = new QLineEdit((fileInfo.*method)());
                make_line_edit_readonly(file_path_text_edit);
                file_paths_formlayout->addRow(copy_file_path_button, file_path_text_edit);
            };
            add(u"path"_s_esc_amp, &QFileInfo::path);
            add(u"filePath"_s_esc_amp, &QFileInfo::filePath);
            add(u"absolutePath"_s_esc_amp, &QFileInfo::absolutePath);
            add(u"absoluteFilePath"_s_esc_amp, &QFileInfo::absoluteFilePath);
            add(u"canonicalPath"_s_esc_amp, &QFileInfo::canonicalPath);
            add(u"canonicalFilePath"_s_esc_amp, &QFileInfo::canonicalFilePath);
            add(u"fileName"_s_esc_amp, &QFileInfo::fileName);
            add(u"baseName"_s_esc_amp, &QFileInfo::baseName);
            add(u"completeSuffix"_s_esc_amp, &QFileInfo::completeSuffix);
            add(u"completeBaseName"_s_esc_amp, &QFileInfo::completeBaseName);
            add(u"suffix"_s_esc_amp, &QFileInfo::suffix);
        }
        menu_widget_consuming_click_vlayout->addLayout(file_paths_formlayout);

        QMenu *menu_menu = new QMenu();
        make_top_level_menu_not_hide_when_toggling_checkable_child_and_not_hide_and_close_some_window_menu_when_clicking_child(menu_menu, [window_menu]
            { window_menu->close(); });
        make_top_level_menu_not_shrink(menu_menu);

        if(isDir)
        {
            QPushButton *directory_menu_clear_cache_button = new QPushButton(u"clear cache"_s_esc_amp);
            QObject::connect(directory_menu_clear_cache_button, &QPushButton::clicked, directory_menu_clear_cache_button, [parentWidget, window_menu, a_in_m]()
                { parentWidget->setProperty("generated", false); window_menu->close();a_in_m->menu()->close(); });
            menu_widget_consuming_click_vlayout->addWidget(directory_menu_clear_cache_button);
        }

        QMenu *file_permissions_menu = new QMenu();
        {
            make_non_top_level_menu_not_hide_when_toggling_checkable_child(file_permissions_menu);
            make_top_level_menu_not_shrink(file_permissions_menu);
            QMap<QString, QVariant> file_permission_menus;
            for(auto [name, p] : FILE_ATTRIBUTE_label_to_value_mapping)
            {
                QAction *file_permission_toggle_action = new QAction(name, file_permissions_menu);
                file_permission_toggle_action->setCheckable(true);
                auto get_file_path = [a_in_m]()
                { return a_in_m->property("absoluteFilePath").toString(); };
                auto on_GetFileAttributesEx_failed = [window_menu](QString error_string)
                { qvariant_cast<QPlainTextEdit *>(window_menu->property("file_permissions_error_string_text_edit"))->appendPlainText(QDateTime::currentDateTime().toString(Qt::DateFormat::ISODateWithMs) + " " + error_string); };
                auto on_SetFileAttributes_failed = [window_menu](QString error_string)
                { qvariant_cast<QPlainTextEdit *>(window_menu->property("file_permissions_error_string_text_edit"))->appendPlainText(QDateTime::currentDateTime().toString(Qt::DateFormat::ISODateWithMs) + " " + error_string); };
                auto on_succeeded = [a_in_m, window_menu]()
                {refresh_action(a_in_m);qvariant_cast<QPlainTextEdit *>(window_menu->property("file_permissions_error_string_text_edit"))->appendPlainText(QDateTime::currentDateTime().toString(Qt::DateFormat::ISODateWithMs)+" "+"GetFileAttributesEx and SetFileAttributes succeeded"); };
                QObject::connect(file_permission_toggle_action, &QAction::toggled, file_permission_toggle_action,
                    [get_file_path, on_GetFileAttributesEx_failed, on_SetFileAttributes_failed, on_succeeded, p](bool checked)
                    {
                        if(WIN32_FILE_ATTRIBUTE_DATA attr; GetFileAttributesEx(string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(get_file_path()).data(), GetFileExInfoStandard, &attr) == 0)
                        {
                            on_GetFileAttributesEx_failed(string_or_string_view_to_string_or_string_view<QString>(GetLastErrorReturnValueToString(GetLastError(), "GetFileAttributesEx")));
                        }
                        else
                        {
                            if(SetFileAttributes(string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(get_file_path()).data(), (attr.dwFileAttributes & ~p) | (checked ? p : 0)) == 0)
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
                file_permission_menus.insert(name, QVariant::fromValue<QObject *>(file_permission_toggle_action));
            }
            window_menu->setProperty("file_permission_menus", file_permission_menus);
        }

        using namespace Qt::Literals::StringLiterals;
        QMenu *file_permissions_window_menu = new QMenu(u"file_permissions"_s_esc_amp, menu_menu);
        {
            make_menu_in_screen(file_permissions_window_menu);
            window_menu->setProperty("file_permissions_window_menu", QVariant::fromValue<QObject *>(file_permissions_window_menu));
            attach_widget_consuming_click(file_permissions_window_menu);
            QVBoxLayout *file_permissions_window_menu_widget_consuming_click_vlayout = new QVBoxLayout(qvariant_cast<QWidget *>(file_permissions_window_menu->property("widget_consuming_click")));
            file_permissions_window_menu_widget_consuming_click_vlayout->addWidget(file_permissions_menu);
            QPlainTextEdit *file_permissions_error_string_text_edit = new QPlainTextEdit();
            make_plain_text_edit_readonly(file_permissions_error_string_text_edit);
            file_permissions_error_string_text_edit->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
            window_menu->setProperty("file_permissions_error_string_text_edit", QVariant::fromValue<QObject *>(file_permissions_error_string_text_edit));
            file_permissions_window_menu_widget_consuming_click_vlayout->addWidget(file_permissions_error_string_text_edit);
            QPushButton *file_permissions_refresh_button = new QPushButton(u"refresh"_s_esc_amp);
            auto refresh = [a_in_m, window_menu]()
            { refresh_window_menu(a_in_m, window_menu); };
            QObject::connect(file_permissions_refresh_button, &QPushButton::clicked, file_permissions_refresh_button, [refresh]()
                { refresh(); });
            file_permissions_window_menu_widget_consuming_click_vlayout->addWidget(file_permissions_refresh_button);
        }
        menu_menu->addMenu(file_permissions_window_menu);

        auto trigger_verb = overloaded{
            [a_in_m, window_menu](QString verb)
            {
                trigger_action(a_in_m, verb);
                window_menu->close();
            },
            [a_in_m, window_menu](std::nullptr_t)
            {
                trigger_action(a_in_m, nullptr);
                window_menu->close();
            },
        };
        QMenu *choose_verbs_menu = new QMenu();
        {
            make_top_level_menu_not_shrink(choose_verbs_menu);
            for(QString verb : {
                    "edit",
                    "explore",
                    "find",
                    "open",
                    "print",
                    "properties",
                    "runas",
                })
            {
                QAction *choose_verb_action = new QAction(verb, choose_verbs_menu);
                QObject::connect(choose_verb_action, &QAction::triggered, choose_verb_action, [trigger_verb, verb]()
                    { trigger_verb(verb); });
                choose_verbs_menu->addAction(choose_verb_action);
            }
            {
                QAction *choose_verb_action = new QAction("(NULL)", choose_verbs_menu);
                QObject::connect(choose_verb_action, &QAction::triggered, choose_verb_action, [trigger_verb]()
                    { trigger_verb(nullptr); });
                choose_verbs_menu->addAction(choose_verb_action);
            }
        }

        QMenu *choose_verbs_window_menu = new QMenu(u"choose_verbs"_s_esc_amp, menu_menu);
        {
            make_menu_in_screen(choose_verbs_window_menu);
            window_menu->setProperty("choose_verbs_window_menu", QVariant::fromValue<QObject *>(choose_verbs_window_menu));
            attach_widget_consuming_click(choose_verbs_window_menu);
            QVBoxLayout *choose_verbs_window_menu_widget_consuming_click_vlayout = new QVBoxLayout(qvariant_cast<QWidget *>(choose_verbs_window_menu->property("widget_consuming_click")));
            choose_verbs_window_menu_widget_consuming_click_vlayout->addWidget(choose_verbs_menu);
            QPlainTextEdit *choose_verbs_text_edit = new QPlainTextEdit();
            choose_verbs_text_edit->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
            QPushButton *choose_verbs_button = new QPushButton(u"trigger"_s_esc_amp);
            QObject::connect(choose_verbs_button, &QPushButton::clicked, choose_verbs_button, [trigger_verb, choose_verbs_text_edit]()
                { trigger_verb(choose_verbs_text_edit->toPlainText()); });
            QHBoxLayout *choose_verbs_custom_hlayout = new QHBoxLayout();
            choose_verbs_window_menu_widget_consuming_click_vlayout->addWidget(choose_verbs_text_edit);
            choose_verbs_window_menu_widget_consuming_click_vlayout->addWidget(choose_verbs_button);
            choose_verbs_window_menu_widget_consuming_click_vlayout->addLayout(choose_verbs_custom_hlayout);
        }
        menu_menu->addMenu(choose_verbs_window_menu);

        menu_widget_consuming_click_vlayout->addWidget(menu_menu);

        refresh_window_menu(a_in_m, window_menu);
        window_menu->setMinimumWidth(window_menu->screen()->geometry().width() * 0.35);
        window_menu->popup(m->mapToGlobal(pos_in_m));
    }
}

void generate_directory_menu_s_children(QMenu *directory_menu, QString directory, bool top_level = false)
{
    QObject::connect(directory_menu->menuAction(), &QAction::hovered, directory_menu->menuAction(), [directory_menu, directory]()
        {
            if(directory_menu->property("generated").toBool())
                return;
            directory_menu->setProperty("generated",true);
            qDeleteAll(directory_menu->actions());

            QFileInfo directory_file_info(directory);
            //        ext_debug_log((info, info.isDir(), info.isSymLink(), info.isSymbolicLink(), info.symLinkTarget()), qDebug_compact());
            QFileInfoList fileInfoList=directory_file_info.isSymLink() ? QFileInfoList({QFileInfo(directory_file_info.symLinkTarget())}) : QDir(directory).entryInfoList(QDir::Filter::NoDot | QDir::Filter::NoDotDot | QDir::Filter::AllEntries | QDir::Filter::Hidden | QDir::Filter::System, QDir::SortFlag::DirsFirst | QDir::SortFlag::Name);
            for(QFileInfo file_file_Info : fileInfoList)
            {
                QAction*a;

                if(file_file_Info.isSymLink() && file_file_Info.isDir())
                {
                    QMenu *subdirectory_menu = new QMenu(directory_menu);
                    a=subdirectory_menu->menuAction();
                    make_menu_not_trigger_child_when_right_click_child(subdirectory_menu,MouseRightButtonPress_handler);
                    make_menu_not_hide_when_not_clicking_on_action(subdirectory_menu);
                    directory_menu->addMenu(subdirectory_menu);
                    generate_directory_menu_s_children(subdirectory_menu, file_file_Info.symLinkTarget());
                }
                else if(file_file_Info.isDir())
                {
                    QMenu *subdirectory_menu = new QMenu(directory_menu);
                    a=subdirectory_menu->menuAction();
                    make_menu_not_trigger_child_when_right_click_child(subdirectory_menu,MouseRightButtonPress_handler);
                    make_menu_not_hide_when_not_clicking_on_action(subdirectory_menu);
                    directory_menu->addMenu(subdirectory_menu);
                    generate_directory_menu_s_children(subdirectory_menu, file_file_Info.absoluteFilePath());
                }
                else
                {
                    QAction *file_action = new QAction(directory_menu);
                    a=file_action;
                    QObject::connect(file_action, &QAction::triggered, file_action, [file_action]()
                        {
                            auto [lpVerb, lpFile, lpParameters, lpDirectory] = std::make_tuple(
                                nullptr,
                                string_or_string_view_to_string_or_string_view<std::basic_string<TCHAR>>(file_action->property("absoluteFilePath").toString()),
                                nullptr,
                                nullptr);
                            if(SHELLEXECUTEINFO execInfo{
                                   .cbSize = sizeof(execInfo),
                                   .fMask = SEE_MASK_INVOKEIDLIST,
                                   .lpVerb = lpVerb,
                                   .lpFile = lpFile.data(),
                                   .lpParameters = lpParameters,
                                   .lpDirectory = lpDirectory,
                                   .nShow = SW_SHOWDEFAULT,
                               };
                                ShellExecuteEx(&execInfo) == FALSE)
                            {
                                assert(!(reinterpret_cast<int>(execInfo.hInstApp) > 32));
                                ext_debug_log((file_action->property("absoluteFilePath").toString(), ShellExecuteEx_error_string(reinterpret_cast<int>(execInfo.hInstApp)), GetLastErrorReturnValueToString(GetLastError(), "ShellExecuteEx")), qDebug_compact());
                            }
                            else
                            {
                                assert(reinterpret_cast<int>(execInfo.hInstApp) > 32);
                            }
                        });
                    directory_menu->addAction(file_action);
                }
                a->setProperty("absoluteFilePath", file_file_Info.absoluteFilePath());
                refresh_action(a);
            }
            if(fileInfoList.empty())
            {
                QAction *file_action = new QAction("(empty)", directory_menu);
                file_action->setEnabled(false);
                directory_menu->addAction(file_action);
            } });
    if(top_level)
        directory_menu->menuAction()->hovered();
};

QWidget *create_start_menu_widget(QMenu *window_menu)
{
    std::vector<std::tuple<QString, QString>> start_menu_directories;
    {
        std::map<std::basic_string<TCHAR>, std::basic_string<TCHAR>> env;
        get_envoronment_variables(std::inserter(env, env.end()));
        //        ext_debug_log((env), qDebug_compact());

        if(auto [contains_ALLUSERSPROFILE, it_ALLUSERSPROFILE] = contains_and_find(env, TEXT("ALLUSERSPROFILE")); contains_ALLUSERSPROFILE)
        {
            start_menu_directories.push_back({string_or_string_view_to_string_or_string_view<QString>(string_literal_to_string_view(TEXT(R"(%ALLUSERSPROFILE%\Microsoft\Windows\Start Menu)"))), string_or_string_view_to_string_or_string_view<QString>(it_ALLUSERSPROFILE->second + TEXT(R"(\Microsoft\Windows\Start Menu)"))});
        }
        if(auto [contains_APPDATA, it_APPDATA] = contains_and_find(env, TEXT("APPDATA")); contains_APPDATA)
        {
            start_menu_directories.push_back({string_or_string_view_to_string_or_string_view<QString>(string_literal_to_string_view(TEXT(R"(%APPDATA%\Microsoft\Windows\Start Menu)"))), string_or_string_view_to_string_or_string_view<QString>(it_APPDATA->second + TEXT(R"(\Microsoft\Windows\Start Menu)"))});
        }

        if(auto [contains_USERSPROFILE, it_USERSPROFILE] = contains_and_find(env, TEXT("USERPROFILE")); contains_USERSPROFILE)
        {
            start_menu_directories.push_back({string_or_string_view_to_string_or_string_view<QString>(string_literal_to_string_view(TEXT(R"(%USERPROFILE%\Desktop)"))), string_or_string_view_to_string_or_string_view<QString>(it_USERSPROFILE->second + TEXT(R"(\Desktop)"))});
        }
    }

    QWidget *start_menu_widget_inner = new QWidget();
    QVBoxLayout *start_menu_vlayout_inner = new QVBoxLayout(start_menu_widget_inner);
    for(auto [directory_schema, directory] : start_menu_directories)
    {
        QMenu *directory_menu = new QMenu();
        make_top_level_menu_not_shrink(directory_menu);
        directory_menu->menuAction()->setProperty("absoluteFilePath", directory);
        make_menu_not_trigger_child_when_right_click_child(directory_menu, MouseRightButtonPress_handler);
        make_menu_not_hide_when_not_clicking_on_action(directory_menu);
        make_top_level_menu_not_hide_when_toggling_checkable_child_and_not_hide_and_close_some_window_menu_when_clicking_child(directory_menu, [window_menu]
            { window_menu->close(); });
        generate_directory_menu_s_children(directory_menu, directory, true);

        GroupBox *directory_groupbox = new GroupBox(escape_ampersand(directory_schema + '\n' + directory));
        QHBoxLayout *directory_hlayout = new QHBoxLayout(directory_groupbox);
        directory_hlayout->addWidget(directory_menu);
        directory_hlayout->addStretch(1);

        start_menu_vlayout_inner->addWidget(directory_groupbox);
    }

    QWidget *start_menu_widget_outer = new QWidget();
    QVBoxLayout *start_menu_vlayout_outer = new QVBoxLayout(start_menu_widget_outer);
    start_menu_vlayout_outer->setContentsMargins(QMargins());
    start_menu_vlayout_outer->addWidget(start_menu_widget_inner);
    start_menu_vlayout_outer->addStretch(1);
    ScrollAreaVertical *start_menu_scroll_area = new ScrollAreaVertical(start_menu_widget_outer);

    return start_menu_scroll_area;
}
