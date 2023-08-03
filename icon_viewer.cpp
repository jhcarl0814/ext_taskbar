#include "icon_viewer.hpp"

#include "ext_infrastructure/ext_ranges.hpp"
#include "ext_infrastructure/ext_string.hpp"

#include "ext_gui/ext_image.hpp"

#include <QLabel>
#include "ext_widgets/flow_layout.hpp"
#include "ext_widgets/scroll_area_vertical.hpp"
#include "ext_widgets/escape_ampersand.hpp"
#include "ext_widgets/ext_groupbox.hpp"

QWidget *create_icon_viewer_widget(QString absoluteFilePath)
{
    QWidget *widget_inner = new QWidget();
    //    FlowLayout *layout_inner = new FlowLayout(widget_inner);
    QVBoxLayout *layout_inner = new QVBoxLayout(widget_inner);
    QWidget *widget_outer = new QWidget();
    QVBoxLayout *vlayout_outer = new QVBoxLayout(widget_outer);
    vlayout_outer->addWidget(widget_inner);
    vlayout_outer->addStretch(1);

    QWidget *widget = new QWidget();
    QVBoxLayout *vlayout = new QVBoxLayout(widget);
    ScrollAreaVertical *scl_quick_launch_groups = new ScrollAreaVertical(widget_outer);
    vlayout->addWidget(scl_quick_launch_groups);

    std::vector<std::vector<QImage>> icons_groups;
    get_images_EnumResourceNames<"icon">(string_or_string_view_to_string_or_string_view<std::basic_string_view<TCHAR>>(absoluteFilePath), std::back_inserter(icons_groups), [&](auto &&)
        { return std::back_inserter(icons_groups.back()); });
    std::vector<std::vector<QImage>> cursors_groups;
    get_images_EnumResourceNames<"cursor">(string_or_string_view_to_string_or_string_view<std::basic_string_view<TCHAR>>(absoluteFilePath), std::back_inserter(cursors_groups), [&](auto &&)
        { return std::back_inserter(cursors_groups.back()); });

    for(auto &images_group : std::views::concat(icons_groups, cursors_groups))
    {
        GroupBox *g = new GroupBox(escape_ampersand(""));
        FlowLayout *layout_group = new FlowLayout(g);
        layout_inner->addWidget(g);
        for(auto &image : images_group)
        {
            QLabel *l = new QLabel();
            QPixmap p = QPixmap::fromImage(image);
            l->setPixmap(p);
            layout_group->addWidget(l);

            QLabel *l2 = new QLabel();
            l2->setText(escape_ampersand(QString::number(p.width()) + ' ' + QString::number(p.height()) + ' ' + QString::number(p.depth())));
            layout_group->addWidget(l2);
        }
    }

    return widget;
}
