#ifndef MESSAGEPROCESSOR_H
#define MESSAGEPROCESSOR_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include "PosixIBClient.h"

class MessageProcessor : public QThread
{
    Q_OBJECT
public:
    MessageProcessor(std::shared_ptr<PosixIBClient> pIBClient, QObject *parent = 0);
    ~MessageProcessor();
public slots:

private:
    void run()
    {
        while (pIBClient->isConnected())
        {
            qDebug() << "MessageProcessor: start() from " << QThread::currentThreadId();
            pIBClient->processMessages();
        }
    }

private:
    std::shared_ptr<PosixIBClient> pIBClient;
};

#endif // MESSAGEPROCESSOR_H
