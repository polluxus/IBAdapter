#include <QtWidgets>

#include "digitalclock.h"

DigitalClock::DigitalClock(QWidget *parent)
    : QLCDNumber(parent)
{


    //pThread = new QThread;
    //pThread->start();
    //this->moveToThread(pThread);

    setWindowFlags(Qt::FramelessWindowHint);
    setSegmentStyle(Flat);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(showTime()));
    timer->start(1000);
    setDigitCount(10);
    showTime();
    resize(180, 30);
}

void DigitalClock::showTime()
{
    QTime time = QTime::currentTime();
    QString text = time.toString("hh:mm:ss");
//    if ((time.second() % 2) == 0)
//        text[2] = ' ';
    //qDebug() << text;

    display(text);
}
