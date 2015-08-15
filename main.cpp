#include <QFile>
//#include "mainwindow.h"
#include "polluxustopbar.h"
#include <QApplication>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    PolluxusTopBar topBar;

    QFile qssFile(":/stylesheets/darkorange.qss");
    qssFile.open(QFile::ReadOnly);
    topBar.setStyleSheet(qssFile.readAll());

    topBar.show();

    return a.exec();
}
