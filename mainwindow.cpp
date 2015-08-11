#include <QtWidgets>
#include <QDebug>
#include "PosixIBClient.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    loadIBSettings();

    QWidget *cWidget = new QWidget;
    setCentralWidget(cWidget);

    QHBoxLayout *hLayout = new QHBoxLayout;

    QLabel *lbMode = new QLabel(tr("Mode:"));
    QStringList adapterList = QStringList() << QString::fromStdString(demoSetting.discstr)
                                            << QString::fromStdString(paperSetting.discstr)
                                            << QString::fromStdString(liveSetting.discstr) ;
    cmbMode = new QComboBox();
    cmbMode->addItems(adapterList);
    cmbMode->setCurrentIndex(0);
    cmbMode->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    btnTest = new QPushButton(tr("test"));

    btnConnect = new QPushButton(tr("Connect"));
    btnConnect->setCheckable(true);

    lbLight = new QLabel();
    lbLight->setFixedWidth(24);
    lbLight->setFixedHeight(24);
    lbLight->setScaledContents( true );
    lbLight->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    lbLight->setPixmap(QPixmap(":/images/red.png"));

    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    hLayout->addWidget(lbMode);
    hLayout->addWidget(cmbMode);
    hLayout->addWidget(spacer);
    hLayout->addWidget(btnTest);
    hLayout->addWidget(btnConnect);
    hLayout->addWidget(lbLight);

    hLayout->setMargin(2);
    //hLayout->addStretch(1);

    QWidget *hWidget = new QWidget;
    hWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    //hWidget->setStyleSheet("background-color:white;");
    hWidget->setLayout(hLayout);

    logEdit = new QPlainTextEdit;
    logEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    logEdit->setReadOnly(true);
    logEdit->setMaximumBlockCount(200);
    logEdit->setPlainText(tr("We are ready"));

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(hWidget);
    vLayout->addWidget(logEdit);
    cWidget->setLayout(vLayout);

    connect(btnTest, SIGNAL(clicked(bool)), this, SLOT(onTest()), Qt::DirectConnection);
    connect(btnConnect, SIGNAL(toggled(bool)), this, SLOT(onConnect()), Qt::DirectConnection);
    connect(cmbMode, SIGNAL(currentIndexChanged(int)), this, SLOT(onAdapterSettingChange(int)));

    setWindowTitle(tr("IBAdaptor"));
    setFixedSize(480, 240);


    pIBAdapter = new PosixIBClient;

    connect(pIBAdapter, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(pIBAdapter, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
}

MainWindow::~MainWindow()
{
    if(pIBAdapter)
    {
        delete pIBAdapter;
    }
}


void MainWindow::loadIBSettings()
{

    QSettings *ibSettings = new QSettings(":/ibadapter.ini",QSettings::IniFormat);

    ibSettings->beginGroup("DEMO");
    demoSetting.port = ibSettings->value("port", 4001).toInt();
    demoSetting.pubport = ibSettings->value("pubport", 5201).toInt();
    demoSetting.subport = ibSettings->value("subport", 5202).toInt();
    demoSetting.discstr = "DEMO - port:" + std::to_string(demoSetting.port) +
                               " - pub:" + std::to_string(demoSetting.pubport) +
                               " - sub:" + std::to_string(demoSetting.subport);

    ibSettings->beginGroup("PAPER");
    paperSetting.port = ibSettings->value("port", 4001).toInt();
    paperSetting.pubport = ibSettings->value("pubport", 5211).toInt();
    paperSetting.subport = ibSettings->value("subport", 5212).toInt();
    paperSetting.discstr = "PAPER - port:" + std::to_string(paperSetting.port) +
                                " - pub:" + std::to_string(paperSetting.pubport) +
                                " - sub:" + std::to_string(paperSetting.subport);

    ibSettings->beginGroup("LIVE");
    liveSetting.port = ibSettings->value("port", 4001).toInt();
    liveSetting.pubport = ibSettings->value("pubport", 5221).toInt();
    liveSetting.subport = ibSettings->value("subport", 5222).toInt();
    liveSetting.discstr = "LIVE - port:" + std::to_string(liveSetting.port) +
                               " - pub:" + std::to_string(liveSetting.pubport) +
                               " - sub:" + std::to_string(liveSetting.subport);

    currSetting = demoSetting;
}

void MainWindow::onAdapterSettingChange(int currentIndex)
{
    switch(currentIndex)
    {
        case 0:
            currSetting = demoSetting;
            break;
        case 1:
            currSetting = paperSetting;
            break;
        case 2:
            currSetting = liveSetting;
            break;
        default:
            break;
    }
}
void MainWindow::onTest()
{
    QMetaObject::invokeMethod(pIBAdapter, "onTest", Qt::QueuedConnection);
}

void MainWindow::onConnect()
{
    if(btnConnect->isChecked())
    {
        qDebug() << "MainWindow:Hi I am connecting ib.------"  << QThread::currentThreadId();

        QMetaObject::invokeMethod(pIBAdapter, "onConnect", Qt::QueuedConnection,
                                  Q_ARG(int, currSetting.port),
                                  Q_ARG(int, currSetting.pubport),
                                  Q_ARG(int, currSetting.subport));

        btnConnect->setText(tr("Connecting..."));
        btnConnect->setEnabled(false);
        cmbMode->setEnabled(false);

        lbLight->setPixmap(QPixmap(":/images/wait.png"));

    }

    else
    {
        qDebug() << "MainWindow:Hi I am disconnecting ib.------"  << QThread::currentThreadId();

        QMetaObject::invokeMethod(pIBAdapter, "onDisconnect", Qt::QueuedConnection );
        cmbMode->setEnabled(false);
        btnConnect->setText(tr("Disconnecting"));
        btnConnect->setEnabled(false);
        lbLight->setPixmap(QPixmap(":/images/wait.png"));

    }
}

void MainWindow::onConnected()
{
    qDebug() << "MainWindow:Recv connected signal from Posix.------"  << QThread::currentThreadId();

    btnConnect->setText(tr("Disconnect"));
    btnConnect->setEnabled(true);
    cmbMode->setEnabled(false);

    int currentMode = cmbMode->currentIndex();
    switch(currentMode)
    {
        case 0:
            lbLight->setPixmap(QPixmap(":/images/yellow.png"));
            break;
        case 1:
            lbLight->setPixmap(QPixmap(":/images/blue.png"));
            break;
        case 2:
            lbLight->setPixmap(QPixmap(":/images/green.png"));
            break;
        default:
            break;
    }
}

void MainWindow::onDisconnected()
{
    qDebug() << "MainWindow:Recv disconnected signal from Posix.------"  << QThread::currentThreadId();

    btnConnect->setText(tr("Connect"));
    btnConnect->setEnabled(true);
    cmbMode->setEnabled(true);
    lbLight->setPixmap(QPixmap(":/images/red.png"));

}


