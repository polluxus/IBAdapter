#include "polluxustopbar.h"

PolluxusTopBar::PolluxusTopBar(QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint)
{

    m_nMouseClick_X_Coordinate = 0;
    m_nMouseClick_Y_Coordinate = 0;

    loadIBSettings();

    createMenuBar();
    createToolBar();


    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setMargin(0);
    hLayout->setContentsMargins(0,0,0,0);


    hLayout->addWidget(pMenuBar);
    hLayout->addWidget(pToolBar);
    setLayout(hLayout);

    adjustTopBarPosition();


    pIBAdapter = new PosixIBClient;

    std::shared_ptr<PosixIBClient> pIBClient(pIBAdapter);
    pMsgProcessor = new MessageProcessor(pIBClient);

    connect(pIBAdapter, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(pIBAdapter, SIGNAL(disconnected()), this, SLOT(onDisconnected()));


    pLogger = new PolluxusLogger(this);
    pLogger->show();

    connect(pIBAdapter, SIGNAL(orderUpdated(QString)), pLogger, SLOT(onOrderUpdated(QString)));
    loadWorkSpace();

}

PolluxusTopBar::~PolluxusTopBar()
{

    saveWorkSpace();
}


void PolluxusTopBar::loadIBSettings()
{
    QString iniFileString = ::getIniFilePath();

    QSettings *wsSettings = new QSettings(iniFileString, QSettings::IniFormat);

    qDebug()<<"testport:"<<wsSettings->value("testport", 2222).toInt();

    wsSettings->beginGroup("DEMO");
    demoSetting.port = wsSettings->value("port", 4001).toInt();
    demoSetting.pubport = wsSettings->value("pubport", 5201).toInt();
    demoSetting.subport = wsSettings->value("subport", 5202).toInt();
    demoSetting.discstr = "DEMO - port:" + std::to_string(demoSetting.port) +
                               " - pub:" + std::to_string(demoSetting.pubport) +
                               " - sub:" + std::to_string(demoSetting.subport);

    wsSettings->beginGroup("PAPER");
    paperSetting.port = wsSettings->value("port", 4001).toInt();
    paperSetting.pubport = wsSettings->value("pubport", 5211).toInt();
    paperSetting.subport = wsSettings->value("subport", 5212).toInt();
    paperSetting.discstr = "PAPER - port:" + std::to_string(paperSetting.port) +
                                " - pub:" + std::to_string(paperSetting.pubport) +
                                " - sub:" + std::to_string(paperSetting.subport);

    wsSettings->beginGroup("LIVE");
    liveSetting.port = wsSettings->value("port", 4001).toInt();
    liveSetting.pubport = wsSettings->value("pubport", 5221).toInt();
    liveSetting.subport = wsSettings->value("subport", 5222).toInt();
    liveSetting.discstr = "LIVE - port:" + std::to_string(liveSetting.port) +
                               " - pub:" + std::to_string(liveSetting.pubport) +
                               " - sub:" + std::to_string(liveSetting.subport);

    currSetting = demoSetting;
}



void PolluxusTopBar::createMenuBar()
{
    pMenuBar = new QMenuBar;

    QAction *quit = new QAction("&Quit", this);
    QAction *saveWS = new QAction("&Save Workspace", this);

    QMenu *file;
    file = pMenuBar->addMenu(QIcon(":/images/setup.png"), "Polluxus");
    file->addAction(quit);
    file->addAction(saveWS);

    pMenuBar->addSeparator();

    connect(quit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(saveWS, SIGNAL(triggered()), this, SLOT(onSaveWorkSpaces()));

    pMenuBar->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
}

void PolluxusTopBar::createToolBar()
{

    pToolBar = new QToolBar;

    QLabel *lbMode = new QLabel(tr("|  Mode:"));
    QStringList adapterList = QStringList() << QString::fromStdString(demoSetting.discstr)
                                            << QString::fromStdString(paperSetting.discstr)
                                            << QString::fromStdString(liveSetting.discstr) ;
    cmbMode = new QComboBox();
    cmbMode->addItems(adapterList);
    cmbMode->setCurrentIndex(0);
    //cmbMode->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    btnTest = new QPushButton(tr("test"));

    btnConnect = new QPushButton(tr("Connect"));
    btnConnect->setCheckable(true);


    DigitalClock *pClock = new DigitalClock(this);



    lbLight = new QLabel();
    lbLight->setFixedWidth(24);
    lbLight->setFixedHeight(24);
    lbLight->setScaledContents( true );
    lbLight->setAlignment(Qt::AlignRight);
    lbLight->setPixmap(QPixmap(":/images/yellow_light.png"));

    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    //pToolBar->addWidget(moveLabel);
    pToolBar->addWidget(lbMode);
    pToolBar->addWidget(cmbMode);

    pToolBar->addWidget(btnTest);
    pToolBar->addWidget(btnConnect);
    pToolBar->addWidget(spacer);
    pToolBar->addWidget(pClock);
    pToolBar->addWidget(lbLight);

    //pClock->show();

    pToolBar->adjustSize();

    pToolBar->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );

    connect(btnTest, SIGNAL(clicked(bool)), this, SLOT(onTest()), Qt::DirectConnection);
    connect(btnConnect, SIGNAL(toggled(bool)), this, SLOT(onConnect()), Qt::DirectConnection);
    connect(cmbMode, SIGNAL(currentIndexChanged(int)), this, SLOT(onAdapterSettingChange(int)));



}

void PolluxusTopBar::adjustTopBarPosition()
{
    resize(QDesktopWidget().availableGeometry().width(), 24);
    move(0, 0);
}



void PolluxusTopBar::onAdapterSettingChange(int currentIndex)
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

void PolluxusTopBar::onTest()
{
    QMetaObject::invokeMethod(pIBAdapter, "onTest", Qt::QueuedConnection);

}

void PolluxusTopBar::onConnect()
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
        lbLight->setPixmap(QPixmap(":/images/bullet-grey.png"));

    }
}

void PolluxusTopBar::onConnected()
{
    qDebug() << "MainWindow:Recv connected signal from Posix.------"  << QThread::currentThreadId();

    btnConnect->setText(tr("Disconnect"));
    btnConnect->setEnabled(true);
    cmbMode->setEnabled(false);

    int currentMode = cmbMode->currentIndex();
    switch(currentMode)
    {
        case 0:
            lbLight->setPixmap(QPixmap(":/images/bullet-yellow.png"));
            break;
        case 1:
            lbLight->setPixmap(QPixmap(":/images/bullet-blue.png"));
            break;
        case 2:
            lbLight->setPixmap(QPixmap(":/images/bullet-green.png"));
            break;
        default:
            break;
    }

    pMsgProcessor->start();
    //QMetaObject::invokeMethod(pIBAdapter, "onReqCurrentTime", Qt::QueuedConnection);
}

void PolluxusTopBar::onDisconnected()
{
    qDebug() << "MainWindow:Recv disconnected signal from Posix.------"  << QThread::currentThreadId();

    btnConnect->setText(tr("Connect"));
    btnConnect->setEnabled(true);
    cmbMode->setEnabled(true);
    lbLight->setPixmap(QPixmap(":/images/bullet-red.png"));

}


void PolluxusTopBar::saveWorkSpace()
{
    qDebug() << "PolluxusTopBar::saveWorkSpace";
    QString iniFileString = ::getIniFilePath();
    QSettings *wsSettings = new QSettings(iniFileString, QSettings::IniFormat);

    wsSettings->setValue("appname", "Polluxus");

    wsSettings->beginGroup("polluxustopbar");
    wsSettings->setValue("geometry", saveGeometry());
    wsSettings->setValue( "maximized", isMaximized());
    if ( !isMaximized() ) {
            wsSettings->setValue( "pos", pos() );
            wsSettings->setValue( "size", size() );
        }
    wsSettings->endGroup();
    wsSettings->sync();
}

void PolluxusTopBar::loadWorkSpace()
{
    qDebug() << "PolluxusTopBar::loadWorkSpace";
    QString iniFileString = ::getIniFilePath();

    QSettings *wsSettings = new QSettings(iniFileString, QSettings::IniFormat);

    int port;
    port = wsSettings->value("appname", 4000).toInt();
    qDebug() << "Read successfully ini:" << port;
    wsSettings->beginGroup("polluxustopbar");
    restoreGeometry(wsSettings->value( "geometry", saveGeometry() ).toByteArray());
    move(wsSettings->value( "pos", pos() ).toPoint());
    resize(wsSettings->value( "size", size()).toSize());
    if ( wsSettings->value( "maximized", isMaximized() ).toBool() )
    {
        showMaximized();
    }
    wsSettings->endGroup();
}

void PolluxusTopBar::onSaveWorkSpaces()
{
    this->saveWorkSpace();
    pLogger->saveWorkSpace();
}
