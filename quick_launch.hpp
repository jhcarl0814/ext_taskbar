#ifndef QUICK_LAUNCH_H
#define QUICK_LAUNCH_H

#include <QImage>
#include <QToolButton>
#include "ext_infrastructure/ext_json.hpp"

struct quick_launch_t;
struct quick_launch_button_t: public json_object_interface<quick_launch_button_t>
{
    struct shell_execute_t: public json_object_interface<shell_execute_t>
    {
        std::optional<QString> verb;
        QString file_path;
        std::optional<QString> parameters;
        std::optional<QString> working_directory;
        int window_show_state;

        QJsonObject to_json_impl() const;
        static shell_execute_t from_json_impl(QJsonObject &obj);

        static std::map<int, QString> const window_show_state_code_to_name;
        static std::map<QString, int> const window_show_state_name_to_code;
        //SW_SHOWNORMAL by default
        //#define SW_HIDE             0
        //#define SW_SHOWNORMAL       1
        //#define SW_NORMAL           1
        //#define SW_SHOWMINIMIZED    2
        //#define SW_SHOWMAXIMIZED    3
        //#define SW_MAXIMIZE         3
        //#define SW_SHOWNOACTIVATE   4
        //#define SW_SHOW             5
        //#define SW_MINIMIZE         6
        //#define SW_SHOWMINNOACTIVE  7
        //#define SW_SHOWNA           8
        //#define SW_RESTORE          9
        //#define SW_SHOWDEFAULT      10
        //#define SW_FORCEMINIMIZE    11
        //#define SW_MAX              11
    };
    struct create_process_t: public json_object_interface<create_process_t>
    {
        QString command_line;
        std::optional<QString> environment_block;
        std::optional<QString> working_directory;
        std::optional<QString> title;

        QJsonObject to_json_impl() const;
        static create_process_t from_json_impl(QJsonObject &obj);
    };
    std::variant<shell_execute_t, create_process_t> shell_execute_or_create_process;

    bool display_text_overriding;
    QString display_text;

    bool icons_overriding;
    std::vector<QImage> icons;

    bool collapsed;

    QJsonObject to_json_impl() const;
    static quick_launch_button_t from_json_impl(QJsonObject &obj);

    static quick_launch_button_t from_file_path(QString file_path);
    void trigger() const;
    void trigger(QString verb) const;
    void trigger(std::nullptr_t) const;

    void refresh_window_menu(QMenu *menu_start);
    void refresh_button(QToolButton *button);
};

struct quick_launch_buttons_group_t: public json_object_interface<quick_launch_buttons_group_t>
{
    QString name;
    std::vector<quick_launch_button_t> buttons;

    QJsonObject to_json_impl() const;
    static quick_launch_buttons_group_t from_json_impl(QJsonObject &obj);
};

struct quick_launch_t: public json_object_interface<quick_launch_t>
{
    std::vector<QString> directories;
    std::vector<quick_launch_buttons_group_t> buttons_groups;

    QJsonObject to_json_impl() const;
    static quick_launch_t from_json_impl(QJsonObject &obj);

    static quick_launch_t from_config_file_and_file_system(QString file_path);
    void to_config_file(QString file_path);

    void refresh_config_file(QWidget *widget);
    void refresh_text_edit(QWidget *widget);
    void refresh_widget_inner(QWidget *widget);
    QWidget *create_widget(QString config_file_file_path);
};

#endif // QUICK_LAUNCH_H
