#include "polluxuslogger.h"

PolluxusLogger::PolluxusLogger(QWidget *parent) : QWidget(parent)
{


    setWindowFlags(Qt::Window);
    createToolBar();
    createMsgTextEdit();

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setSpacing(0);
    vLayout->setMargin(0);
    vLayout->setContentsMargins(0,0,0,0);

    vLayout->addWidget(pToolBar);
    vLayout->addWidget(pMsgTextEdit);

    setLayout(vLayout);
    setMinimumSize(320, 200);

    loadWorkSpace();

}

PolluxusLogger::~PolluxusLogger()
{
    saveWorkSpace();
}


void PolluxusLogger::createToolBar()
{
    pToolBar = new QToolBar;
    btnClear = new QPushButton(tr("Clear"));
    pToolBar->addWidget(btnClear);
}

void PolluxusLogger::createMsgTextEdit()
{
    pMsgTextEdit = new QPlainTextEdit;
    pMsgTextEdit->setReadOnly(true);
    pMsgTextEdit->appendPlainText("We are ready");

}


void PolluxusLogger::onOrderUpdated(QString msg)
{
    pMsgTextEdit->appendPlainText(msg);
}

void PolluxusLogger::saveWorkSpace()
{
    qDebug() << "PolluxusLogger::saveWorkSpace";
    QString iniFileString = ::getIniFilePath();

    QSettings *wsSettings = new QSettings(iniFileString, QSettings::IniFormat);

    wsSettings->beginGroup("polluxuslogger");
    wsSettings->setValue("geometry", saveGeometry());
    wsSettings->setValue( "maximized", isMaximized());
    if ( !isMaximized() ) {
            wsSettings->setValue( "pos", pos() );
            wsSettings->setValue( "size", size() );
        }
    wsSettings->endGroup();

    wsSettings->sync();
}

void PolluxusLogger::loadWorkSpace()
{
    qDebug() << "PolluxusLogger::loadWorkSpace";
    QString iniFileString = ::getIniFilePath();

    QSettings *wsSettings = new QSettings(iniFileString, QSettings::IniFormat);

    wsSettings->beginGroup("polluxuslogger");
    restoreGeometry(wsSettings->value( "geometry", saveGeometry() ).toByteArray());
    move(wsSettings->value( "pos", pos() ).toPoint());
    resize(wsSettings->value( "size", size()).toSize());
    if ( wsSettings->value( "maximized", isMaximized() ).toBool() )
    {
        showMaximized();
    }
    wsSettings->endGroup();
}
