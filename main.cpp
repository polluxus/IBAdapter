#include <QFile>
#include "polluxustopbar.h"
#include <QApplication>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Polluxus");
    QCoreApplication::setOrganizationDomain("polluxuscapital.com");
    QCoreApplication::setApplicationName("ptrader");

    PolluxusTopBar topBar;

    QFile qssFile(":/stylesheets/darkorange.qss");
    qssFile.open(QFile::ReadOnly);
    topBar.setStyleSheet(qssFile.readAll());

    topBar.show();

    return a.exec();
}
