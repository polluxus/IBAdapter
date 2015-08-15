#ifndef POLLUXUSTOPBAR_H
#define POLLUXUSTOPBAR_H

#include <QObject>
#include <QWidget>
#include <QtWidgets>
#include <QDesktopWidget>
#include <QDebug>
#include "ibstructs.h"
#include "PosixIBClient.h"
#include "messageprocessor.h"

class PolluxusTopBar : public QWidget
{
    Q_OBJECT
public:

    PosixIBClient *pIBAdapter;
    MessageProcessor* pMsgProcessor;

    explicit PolluxusTopBar(QWidget *parent = 0);

    void loadIBSettings();

    void mousePressEvent(QMouseEvent* event)
    {
        //qDebug() << "mousePressEvent()";
        m_nMouseClick_X_Coordinate = event->x();
        m_nMouseClick_Y_Coordinate = event->y();
    };

    void mouseMoveEvent(QMouseEvent* event)
    {
        //qDebug() << "mouseMoveEvent()";
        move(event->globalX()-m_nMouseClick_X_Coordinate,event->globalY()-m_nMouseClick_Y_Coordinate);
    };

    void mouseReleaseEvent(QMouseEvent* event)
    {
        //qDebug() << "mouseReleaseEvent()";
        if(event->globalY() < 100)
        {
            move(-(event->globalX()),-(event->globalY()));
        }
    };



private:

    int  m_nMouseClick_X_Coordinate;
    int  m_nMouseClick_Y_Coordinate;


    QMenuBar *pMenuBar;
    QToolBar *pToolBar;

    QPushButton *btnTest;
    QPushButton *btnConnect;
    QComboBox *cmbMode;
    QLabel *lbLight;


    AdapterSetting demoSetting;
    AdapterSetting paperSetting;
    AdapterSetting liveSetting;
    AdapterSetting currSetting;

    void createMenuBar();
    void createToolBar();
    void adjustTopBarPosition();


signals:

public slots:

    void onConnect();
    void onConnected();
    void onDisconnected();
    void onAdapterSettingChange(int currentIndex);
    void onTest();


};

#endif // POLLUXUSTOPBAR_H
