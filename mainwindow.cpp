#include "mainwindow.hpp"

#include <QTimer>

#include <QGuiApplication>
#include <QScreen>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include "ext_widgets/keep_window_stays_on_top.hpp"
#include "ext_widgets/menu_window.hpp"
#include "ext_widgets/menu_positioned.hpp"
#include "ext_widgets/escape_ampersand.hpp"
#include "ext_widgets/ext_button.hpp"

#include "start_menu.hpp"
#include "mime_viewer.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      quick_launch(quick_launch_t::from_config_file_and_file_system("quick_launch.json"))
{
    quick_launch.to_config_file("quick_launch.json");

    //transparent to render
    setWindowFlags(windowFlags() | Qt::WindowType::FramelessWindowHint);
    setAttribute(Qt::WidgetAttribute::WA_TranslucentBackground); // https://doc.qt.io/qt-6/qt.html Setting this flag causes WA_NoSystemBackground to be set. On Windows the widget also needs the Qt::FramelessWindowHint window flag to be set.
    //    setWindowState(Qt::WindowState::WindowFullScreen);

    //    //transparent to mouse
    //    setWindowFlags(windowFlags() | Qt::WindowType::WindowTransparentForInput); //setAttribute(Qt::WidgetAttribute::WA_TransparentForMouseEvents);

    //the window above taskbar when "the last time the window gets active" is after "the last time taskbar gets active"
    setWindowFlags(windowFlags() | Qt::WindowType::WindowStaysOnTopHint);

    keep_window_stays_on_top(this);


    PushButton *window_button = new PushButton(u""_s_esc_amp);
    {
        QSizePolicy p = window_button->sizePolicy();
        p.setVerticalPolicy(QSizePolicy::Policy::Minimum);
        window_button->setSizePolicy(p);
    }
    {
        QTimer *timer = new QTimer(window_button);
        timer->setInterval(1000);
        timer->setTimerType(Qt::TimerType::PreciseTimer);
        connect(timer, &QTimer::timeout, window_button, [window_button]()
            { window_button->setText(escape_ampersand(QDate::currentDate().toString("yy-MM-dd") + '\n' + QDate::currentDate().toString("dddd") + '\n' + QTime::currentTime().toString(Qt::DateFormat::ISODate))); });
        timer->start();
    }
    window_menu = new QMenu(window_button);
    make_menu_button_menu(window_menu, window_button);
    attach_widget_consuming_click(window_menu);
    QHBoxLayout *window_menu_widget_consuming_click_hlayout = new QHBoxLayout(qvariant_cast<QWidget *>(window_menu->property("widget_consuming_click")));

    QVBoxLayout *start_menu_vlayout = new QVBoxLayout();
    QPushButton *start_menu_refresh_button = new QPushButton(u"refresh"_s_esc_amp);
    connect(start_menu_refresh_button, &QPushButton::clicked, start_menu_refresh_button, [this]()
        {
        QWidget*start_menu_widget_new = create_start_menu_widget(window_menu);
          delete start_menu_widget->parentWidget()->layout()->replaceWidget(start_menu_widget, start_menu_widget_new);
          start_menu_widget->deleteLater();
          start_menu_widget=start_menu_widget_new; });
    start_menu_vlayout->addWidget(start_menu_refresh_button);
    start_menu_vlayout->addWidget(start_menu_widget = create_start_menu_widget(window_menu));
    window_menu_widget_consuming_click_hlayout->addLayout(start_menu_vlayout);
    window_menu_widget_consuming_click_hlayout->addWidget(quick_launch_widget = quick_launch.create_widget("quick_launch.json"));
    window_menu_widget_consuming_click_hlayout->addWidget(mime_viewer_widget = create_mime_viewer_widget());

    QWidget *central_widget = new QWidget;
    {
        QPalette pal = central_widget->palette();
        pal.setColor(QPalette::Window, Qt::GlobalColor::white);
        central_widget->setPalette(pal);
    }
    {
        //        set_widget_opacity(central_widget, 0.1);
    }
    QHBoxLayout *central_hlayout = new QHBoxLayout(central_widget);
    central_hlayout->setContentsMargins(QMargins());
    central_hlayout->addWidget(window_button);
    central_hlayout->addStretch(1);
    setCentralWidget(central_widget);

    connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, &MainWindow::on_primaryScreenChanged);
    on_primaryScreenChanged(this->screen());
}

MainWindow::~MainWindow()
{
    //    declare_ext_debug_guard((), qDebug_compact());
}

void MainWindow::showEvent(QShowEvent *event)
{
    //    declare_ext_debug_guard((event), qDebug_compact());
    QMainWindow::showEvent(event);
    refresh_geometry();
}
void MainWindow::hideEvent(QHideEvent *event)
{
    //    declare_ext_debug_guard((event), qDebug_compact());
    QMainWindow::hideEvent(event);
    refresh_geometry();
}
void MainWindow::resizeEvent(QResizeEvent *event)
{
    //    declare_ext_debug_guard((event), qDebug_compact());
    QMainWindow::resizeEvent(event);
    refresh_geometry();
}
void MainWindow::moveEvent(QMoveEvent *event)
{
    //    declare_ext_debug_guard((event), qDebug_compact());
    QMainWindow::moveEvent(event);
    refresh_geometry();
}
void MainWindow::on_primaryScreenChanged(QScreen * /*screen*/)
{
    //          declare_ext_debug_guard((screen), qDebug_compact());
    refresh_geometry(); //
    disconnect(connection_screen_geometry_changed);
    disconnect(connection_screen_available_geometry_changed);
    connection_screen_geometry_changed = connect(this->screen(), &QScreen::geometryChanged, this, [this](QRect const & /*geometry*/)
        {
            //                declare_ext_debug_guard((geometry), qDebug_compact());
            refresh_geometry(); //
        });
    connection_screen_available_geometry_changed = connect(this->screen(), &QScreen::geometryChanged, this, [this](QRect const & /*geometry*/)
        {
            //                declare_ext_debug_guard((geometry), qDebug_compact());
            refresh_geometry(); //
        }); //
}
void MainWindow::refresh_geometry()
{
    //    declare_ext_debug_guard((), qDebug_compact());
    QRect screenGeometry = screen()->geometry();
    QRect screenAvailableGeometry = screen()->availableGeometry();

    setGeometry(QRect(screenGeometry.left(), screenAvailableGeometry.top() + screenAvailableGeometry.height(), screenGeometry.width(), (screenGeometry.top() + screenGeometry.height()) - (screenAvailableGeometry.top() + screenAvailableGeometry.height()))); //setGeometry(QRect(screenAvailableGeometry.bottomLeft() + QPoint(0, 1), screenGeometry.bottomRight()));
    //    window_menu->setMinimumHeight(screenGeometry.height() * 0.6);
    window_menu->setFixedHeight(screenAvailableGeometry.height());
    quick_launch_widget->setMinimumWidth(screenAvailableGeometry.width() * 0.35);
    mime_viewer_widget->setMinimumWidth(screenAvailableGeometry.width() * 0.35);
    window_menu->setMaximumWidth(screenAvailableGeometry.width());

    //    ext_debug_log((screenGeometry, screenGeometry.topLeft(), screenGeometry.bottomRight()), qDebug_compact());
    //    ext_debug_log((screenAvailableGeometry, screenAvailableGeometry.topLeft(), screenAvailableGeometry.bottomRight()), qDebug_compact());
    //    ext_debug_log((geometry(), geometry().topLeft(), geometry().bottomRight()), qDebug_compact());
}
