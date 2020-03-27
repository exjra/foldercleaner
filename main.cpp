#include "mainwindow.h"
#include <QApplication>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("AydoLabs");
    QCoreApplication::setOrganizationDomain("aydolabs.com");
    QCoreApplication::setApplicationName("FolderCleaner");
    QCoreApplication::setApplicationVersion("1.1.0");

    MainWindow w;
    w.show();

    return a.exec();
}
