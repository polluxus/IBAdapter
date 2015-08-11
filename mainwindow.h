#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ibstructs.h"
#include <QMainWindow>

class QAction;
class QLabel;
class QComboBox;
class QPushButton;
class QPlainTextEdit;
class PosixIBClient;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow(); 
    PosixIBClient *pIBAdapter;
    void loadIBSettings();
private:
    QPushButton *btnTest;
    QPushButton *btnConnect;
    QComboBox *cmbMode;
    QLabel *lbLight;
    QPlainTextEdit *logEdit;

    AdapterSetting demoSetting;
    AdapterSetting paperSetting;
    AdapterSetting liveSetting;
    AdapterSetting currSetting;

public slots:
    void onConnect();
    void onConnected();
    void onDisconnected();
    void onAdapterSettingChange(int currentIndex);
    void onTest();
};

#endif // MAINWINDOW_H
