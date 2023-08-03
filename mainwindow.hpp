#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include "quick_launch.hpp"

class MainWindow: public QMainWindow
{
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void moveEvent(QMoveEvent *event) override;

  private:
    void on_primaryScreenChanged(QScreen *screen);
    void refresh_geometry();
    QMetaObject::Connection connection_screen_geometry_changed;
    QMetaObject::Connection connection_screen_available_geometry_changed;
    QMenu *window_menu;

    QWidget *start_menu_widget;

    quick_launch_t quick_launch;
    QWidget *quick_launch_widget;

    QWidget *mime_viewer_widget;
};

#endif // MAINWINDOW_H
