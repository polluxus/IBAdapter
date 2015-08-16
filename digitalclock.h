#ifndef DIGITALCLOCK_H
#define DIGITALCLOCK_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QLCDNumber>

class DigitalClock : public QLCDNumber
{
    Q_OBJECT

public:
    DigitalClock(QWidget *parent = 0);
private:
    //QThread *pThread;

private slots:

    void showTime();
};

#endif // DIGITALCLOCK_H
