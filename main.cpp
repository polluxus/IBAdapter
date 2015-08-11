#include <QFile>
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    MainWindow w;

    QFile qssFile(":/stylesheets/darkorange.qss");
    qssFile.open(QFile::ReadOnly);
    w.setStyleSheet(qssFile.readAll());

    w.show();

    return a.exec();
}
