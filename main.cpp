#include "mainwindow.hpp"

//#define NOMINMAX
#include <windows.h>

#include "ext_infrastructure/ext_debug.hpp"

#include <QScopeGuard>
#include <QThread>

#include <QFileSystemModel>
#include <QFontDatabase>

#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QStyleFactory>
#include <QFileIconProvider>
#include "ext_widgets/ext_style.hpp"

//#include "icon_viewer.hpp"

int main(int argc, char *argv[])
{
    //    ext_debug::ext_debug_test();
    //    //    ShellExecute(NULL, NULL, TEXT("control.exe"), TEXT("access.cpl"), NULL, SW_SHOW);
    //    ShellExecute(NULL, NULL, TEXT("control.exe"), TEXT(""), NULL, SW_SHOW);
    //    WinExec(R"(%systemroot%\system32\control.exe /name Microsoft.WindowsUpdate)", SW_SHOWNORMAL);

    static_assert(__cplusplus == 202004L);

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    QScopeGuard guard([]
        { CoUninitialize(); });

    QApplication a(argc, argv);
    if(QStyleFactory::keys().contains("Windows"))
        qApp->setStyle(QStyleFactory::create("Windows"));
    qApp->setStyle(new MyProxyStyle(qApp->style()));
    {
        //        qreal pointSizeF_old = a.font().pointSizeF();
        //        ext_debug_log((a.font().family(), a.font().toString(), a.font().pixelSize(), a.font().pointSize(), a.font().pointSizeF()), qDebug_compact());
        int id = QFontDatabase::addApplicationFont(":/fonts/unifont-15.0.01.ttf"); // https://stackoverflow.com/questions/30973781/qt-add-custom-font-from-resource
        QString family = QFontDatabase::applicationFontFamilies(id).at(0);
        QFont unifont(family);
        unifont.setStyleHint(QFont::StyleHint::Monospace);
        //        unifont.setPointSizeF(pointSizeF_old);
        unifont.setPointSizeF(10);
        a.setFont(unifont);
        //        ext_debug_log((a.font().family(), a.font().toString(), a.font().pixelSize(), a.font().pointSize(), a.font().pointSizeF()), qDebug_compact());
    }

    {
        QImage placeholder_image(QSize(32, 32), QImage::Format::Format_ARGB32);
        placeholder_image.fill(QColorConstants::Transparent);
        QPixmap placeholder_pixmap = QPixmap::fromImage(placeholder_image);
        QIcon placeholder_icon;
        placeholder_icon.addPixmap(placeholder_pixmap);
        placeholder_icon.addPixmap(placeholder_pixmap, QIcon::Mode::Disabled);
        qApp->setProperty("placeholder_icon", placeholder_icon);
    }
    qApp->setProperty("get_icon_file_icon_provider", QVariant::fromValue<void *>(new QFileIconProvider()));
    qApp->setProperty("get_icon_file_system_model", QVariant::fromValue<QObject *>(new QFileSystemModel())); //https://forum.qt.io/topic/62866/getting-icon-from-external-applications
    {
        QThread *retrieve_icon_thread = new QThread(qApp);
        retrieve_icon_thread->start();
        qApp->setProperty("retrieve_icon_thread", QVariant::fromValue<QObject *>(retrieve_icon_thread));
    }
    {
        QThread *convert_string_thread = new QThread(qApp);
        convert_string_thread->start();
        qApp->setProperty("convert_string_thread", QVariant::fromValue<QObject *>(convert_string_thread));
    }

    if constexpr(true)
    {
        //        create_icon_viewer_widget(string_or_string_view_to_string_or_string_view<QString>(string_literal_to_string_view(TEXT(R"(C:\Users\jhcar\Downloads\shell32.dll)"))))->show();
        //        create_icon_viewer_widget(string_or_string_view_to_string_or_string_view<QString>(string_literal_to_string_view(TEXT(R"(%SystemRoot%\system32\shell32.dll)"))))->show();
        //        create_icon_viewer_widget(string_or_string_view_to_string_or_string_view<QString>(string_literal_to_string_view(TEXT(R"(C:\Windows\System32\appwiz.cpl)"))))->show();
    }

    MainWindow w;
    w.show();

    auto return_value = a.exec();

    {
        QThread *convert_string_thread = qvariant_cast<QThread *>(qApp->property("convert_string_thread"));
        qApp->setProperty("convert_string_thread", QVariant());
        convert_string_thread->quit();
        convert_string_thread->wait();
        delete convert_string_thread;
    }
    {
        QThread *retrieve_icon_thread = qvariant_cast<QThread *>(qApp->property("retrieve_icon_thread"));
        qApp->setProperty("retrieve_icon_thread", QVariant());
        retrieve_icon_thread->quit();
        retrieve_icon_thread->wait();
        delete retrieve_icon_thread;
    }
    {
        QFileSystemModel *get_icon_file_system_model = qvariant_cast<QFileSystemModel *>(qApp->property("get_icon_file_system_model"));
        qApp->setProperty("get_icon_file_system_model", QVariant());
        delete get_icon_file_system_model;
    }
    {
        QFileIconProvider *get_icon_file_icon_provider = static_cast<QFileIconProvider *>(qvariant_cast<void *>(qApp->property("get_icon_file_icon_provider")));
        qApp->setProperty("get_icon_file_icon_provider", QVariant());
        delete get_icon_file_icon_provider;
    }
    qApp->setProperty("placeholder_icon", QVariant());

    return return_value;
}
