#-------------------------------------------------
#
# Project created by QtCreator 2015-08-07T21:40:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = IBAdapter
TEMPLATE = app

CONFIG += c++11
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter

SOURCES += main.cpp\
        mainwindow.cpp \
    tws/src/EClientSocketBase.cpp \
    tws/src/EPosixClientSocket.cpp \
    PosixIBClient.cpp

HEADERS  += mainwindow.h \
    tws/Shared/CommissionReport.h \
    tws/Shared/CommonDefs.h \
    tws/Shared/Contract.h \
    tws/Shared/EClient.h \
    tws/Shared/EClientSocketBase.h \
    tws/Shared/EClientSocketBaseImpl.h \
    tws/Shared/EWrapper.h \
    tws/Shared/Execution.h \
    tws/Shared/IBString.h \
    tws/Shared/Order.h \
    tws/Shared/OrderState.h \
    tws/Shared/ScannerSubscription.h \
    tws/Shared/shared_ptr.h \
    tws/Shared/StdAfx.h \
    tws/Shared/TagValue.h \
    tws/Shared/TwsSocketClientErrors.h \
    tws/src/EPosixClientSocket.h \
    tws/src/EPosixClientSocketPlatform.h \
    PosixIBClient.h \
    ibstructs.h

INCLUDEPATH += tws/src \
               tws/Shared

INCLUDEPATH += $$PWD/../../../../../usr/local/include \
               ../resources/images \
               ../resources/stylesheets

unix:!macx: LIBS += -L$$PWD/../../../../../usr/local/lib/ -lzmq


DEPENDPATH += $$PWD/../../../../../usr/local/include

RESOURCES += \
    ../resources/resources.qrc





