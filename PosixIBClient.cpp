/* Copyright (C) 2013 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */
#include <stdio.h>
#include <QDebug>
#include "PosixIBClient.h"

#include "EPosixClientSocket.h"
#include "EPosixClientSocketPlatform.h"

#include "Contract.h"
#include "Order.h"

const int PING_DEADLINE = 2; // seconds
const int SLEEP_BETWEEN_PINGS = 30; // seconds

///////////////////////////////////////////////////////////
// member funcs
PosixIBClient::PosixIBClient(QObject *parent) : QObject(parent) //QObject *parent   : QObject(parent)
{
    state = ST_CONNECT;
    sleepDeadline = 0;
    nextOrderId = static_cast<long int> (time(NULL));

    pThread.reset(new QThread);
    pThread->start();
    this->moveToThread(pThread.get());

    pClient.reset(new EPosixClientSocket(this));
}

PosixIBClient::~PosixIBClient()
{
    pClient.release();
    pThread->exit( 0 );
    pThread->wait();
    pThread.release();

}


bool PosixIBClient::connect(const char *host, unsigned int port, int clientId)
{
    // trying to connect
    bool bRes = pClient->eConnect( host, port, clientId);
    if(bRes)
    {
        qDebug() << "PosixIBClient: connected to server";
    }
    else
    {
        qDebug() << "PosixIBClient: failed to connect to server";
    }
    return bRes;
}

void PosixIBClient::disconnect() const
{
    pClient->eDisconnect();
    qDebug() << "PosixIBClient: disconnected to server";

}

bool PosixIBClient::isConnected() const
{
    return pClient->isConnected();
}


void PosixIBClient::onConnect(int port,  int pubport,  int subport)
{

    qDebug() << "PosixIBClient: onConnect() from " << QThread::currentThreadId();
    this->port = port;
    this->pubport = pubport;
    this->subport = subport;

    bool bConn = false;
    int attemptCount = 0;

    while(attemptCount <= 10)
    {
        ++attemptCount;
        qDebug() << "Connecting to IB, No of try:" << attemptCount ;
        bConn = connect("127.0.0.1", port, 0);

        if(bConn)
        {

            break;
        }

        sleep(5);

    }

    if(bConn)
    {
        qDebug() << "Connected------------------------------.";
        emit connected();
        //onProcessMessages();
    }
    else
    {

        qDebug() << "Failed to connect. Max no of try reached.";
        emit disconnected();
    }
}

void PosixIBClient::onDisconnect()
{
    qDebug() <<"PosixIBClient::onDisconnect() entered";

    disconnect();

    while(isConnected())
    {
        sleep(2);
    }

    qDebug() <<"Disonnected";
    emit disconnected();
}

void PosixIBClient::onProcessMessages()
{
    while (isConnected())
    {
        processMessages();
    }

}

void PosixIBClient::processMessages()
{
    //qDebug() << "PosixIBClient: processMessages() from " << QThread::currentThreadId();

    fd_set readSet, writeSet, errorSet;

    struct timeval tval;
    tval.tv_usec = 0;
    tval.tv_sec = 0;

    time_t now = time(NULL);

    if( sleepDeadline > 0) {
        // initialize timeout with m_sleepDeadline - now
        qDebug() << "PosixClient::processMessages";
        tval.tv_sec = sleepDeadline - now;
    }

    if( pClient->fd() >= 0 )
    {


        FD_ZERO( &readSet);
        writeSet = readSet;

        FD_SET( pClient->fd(), &readSet);

        if( !pClient->isOutBufferEmpty())
            FD_SET( pClient->fd(), &writeSet);

        FD_SET( pClient->fd(), &errorSet);

        int ret = select( pClient->fd() + 1, &readSet, &writeSet, NULL, &tval);

        if( ret == 0) { // timeout
            //qDebug() <<"PosixClient::processMessages: timeout";
            return;
        }

        if( ret < 0) {	// error
           //qDebug() <<"PosixClient::processMessages: disconnect";
            disconnect();

            return;
        }

        if( pClient->fd() < 0)
        {
            //qDebug() << "PosixClient::processMessages, fd() < 0, return b4 writeset";
            return;
        }

        if( FD_ISSET( pClient->fd(), &writeSet)) {
            // socket is ready for writing
            //qDebug() << "PosixClient::processMessages: onSend()";
            pClient->onSend();
        }

        if( pClient->fd() < 0)
        {
            //qDebug() << "PosixClient::processMessages, fd() < 0, return b4 readSet";
            return;
        }

        if( FD_ISSET( pClient->fd(), &readSet)) {
            // socket is ready for reading
            //qDebug() << "PosixClient::processMessages: onReceive()";
            pClient->onReceive();
        }


    }

}

void PosixIBClient::onTest()
{

    qDebug() << "Placing order ....";
    placeOrder();

    //qDebug() << "Requesting market data ....";
    //onReqMktData();
}


void PosixIBClient::sayHello()
{
    qDebug() <<"Hello from PosixIBClient";
    qDebug() << QThread::currentThreadId();
}



void PosixIBClient::onReqCurrentTime()
{
    qDebug() << "onReqCurrentTime() entered";
}

void PosixIBClient::onReqMktData()
{
    Contract contract;


    contract.symbol = "ES";
    contract.secType = "FUT";
    contract.exchange = "GLOBEX";
    contract.expiry = "201509";
    contract.currency = "USD";


    qDebug() << "I am going to reqMktData";
    pClient->reqMktData(1, contract, "", false, TagValueListSPtr());

}

void PosixIBClient::onCancelMktData()
{
    pClient->cancelMktData(1);
}

void PosixIBClient::getNextValidID()
{
    nextOrderId += 1;
}

//////////////////////////////////////////////////////////////////
// methods
void PosixIBClient::reqCurrentTime()
{
//    qDebug() << "PosixIBClient: requesting current time";
//    sleepDeadline = time( NULL) + PING_DEADLINE;
//    state = ST_PING_ACK;
//    pClient->reqCurrentTime();

}

void PosixIBClient::placeOrder()
{
	Contract contract;
	Order order;

    contract.symbol = "ES";
    contract.secType = "FUT";
    contract.exchange = "GLOBEX";
    contract.expiry = "201509";
	contract.currency = "USD";

    order.action = "BUY";
    order.totalQuantity = 5;
	order.orderType = "LMT";
    order.lmtPrice = 2045.00;

    qDebug() << "Placing order using ID:" << nextOrderId;
    pClient->placeOrder(nextOrderId, contract, order);
    getNextValidID();
}

void PosixIBClient::cancelOrder()
{

}

///////////////////////////////////////////////////////////////////
// events
void PosixIBClient::orderStatus( OrderId orderId, const IBString &status, int filled,
	   int remaining, double avgFillPrice, int permId, int parentId,
	   double lastFillPrice, int clientId, const IBString& whyHeld)

{
    QString msg("OrderStatus | orderId:");
    msg.append(QString::number(orderId));
    msg.append(", Status:");
    msg.append(QString::fromStdString(status));
    msg.append(", remaining:");
    msg.append(QString::number(filled));
    msg.append(", filled:");
    msg.append(QString::number(remaining));
    msg.append(", avgPx:");
    msg.append(QString::number(avgFillPrice));

//    msg =  "OrderStatus:" + "OrderId:" + orderId + ", Status:"
//                          + QString::fromStdString(status) + ", filled:" + filled
//                          + ", remaining:" + remaining + ", avgPx:" + avgFillPrice
//                          + ", permId:" + permId + ", parentId:" + parentId
//                          + ", lastFillPrice:" + lastFillPrice
//                          + ", clientId:" + clientId
//                          + ", whyHeld:" + QString::fromStdString(whyHeld);

    qDebug() << msg;

    emit orderUpdated(msg);
}

void PosixIBClient::nextValidId( OrderId orderId)
{
    //qDebug() << "Received nextValidId:" << orderId;
    //nextOrderId = orderId;
}

void PosixIBClient::currentTime( long time)
{
//    if (state == ST_PING_ACK) {
//        time_t t = ( time_t)time;

//        QDateTime dt;
//        dt.setTime_t(t);

//        qDebug() << "IB currentTime:" << dt.toString("hh:mm:ss:zzz");

//        time_t now = ::time(NULL);
//        sleepDeadline = now + SLEEP_BETWEEN_PINGS;

//        state = ST_IDLE;
//    }


}

void PosixIBClient::error(const int id, const int errorCode, const IBString errorString)
{

	if( id == -1 && errorCode == 1100) // if "Connectivity between IB and TWS has been lost"
		disconnect();
}

void PosixIBClient::tickPrice( TickerId tickerId, TickType field, double price, int canAutoExecute)
{
    qDebug() << "Tick price:" << tickerId << ", price:" << price;

}
void PosixIBClient::tickSize( TickerId tickerId, TickType field, int size)
{
    qDebug() << "Tick size:" << tickerId << ", size:" << size;

}
void PosixIBClient::tickOptionComputation( TickerId tickerId, TickType tickType, double impliedVol, double delta,
											 double optPrice, double pvDividend,
											 double gamma, double vega, double theta, double undPrice) {}
void PosixIBClient::tickGeneric(TickerId tickerId, TickType tickType, double value) {}
void PosixIBClient::tickString(TickerId tickerId, TickType tickType, const IBString& value) {}
void PosixIBClient::tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const IBString& formattedBasisPoints,
							   double totalDividends, int holdDays, const IBString& futureExpiry, double dividendImpact, double dividendsToExpiry) {}
void PosixIBClient::openOrder( OrderId orderId, const Contract&, const Order&, const OrderState& ostate) {}
void PosixIBClient::openOrderEnd() {}
void PosixIBClient::winError( const IBString &str, int lastError) {}
void PosixIBClient::connectionClosed(){}
void PosixIBClient::updateAccountValue(const IBString& key, const IBString& val,
										  const IBString& currency, const IBString& accountName) {}
void PosixIBClient::updatePortfolio(const Contract& contract, int position,
		double marketPrice, double marketValue, double averageCost,
		double unrealizedPNL, double realizedPNL, const IBString& accountName){}
void PosixIBClient::updateAccountTime(const IBString& timeStamp) {}
void PosixIBClient::accountDownloadEnd(const IBString& accountName) {}
void PosixIBClient::contractDetails( int reqId, const ContractDetails& contractDetails) {}
void PosixIBClient::bondContractDetails( int reqId, const ContractDetails& contractDetails) {}
void PosixIBClient::contractDetailsEnd( int reqId) {}
void PosixIBClient::execDetails( int reqId, const Contract& contract, const Execution& execution) {}
void PosixIBClient::execDetailsEnd( int reqId) {}

void PosixIBClient::updateMktDepth(TickerId id, int position, int operation, int side,
									  double price, int size) {}
void PosixIBClient::updateMktDepthL2(TickerId id, int position, IBString marketMaker, int operation,
										int side, double price, int size) {}
void PosixIBClient::updateNewsBulletin(int msgId, int msgType, const IBString& newsMessage, const IBString& originExch) {}
void PosixIBClient::managedAccounts( const IBString& accountsList) {}
void PosixIBClient::receiveFA(faDataType pFaDataType, const IBString& cxml) {}
void PosixIBClient::historicalData(TickerId reqId, const IBString& date, double open, double high,
									  double low, double close, int volume, int barCount, double WAP, int hasGaps) {}
void PosixIBClient::scannerParameters(const IBString &xml) {}
void PosixIBClient::scannerData(int reqId, int rank, const ContractDetails &contractDetails,
	   const IBString &distance, const IBString &benchmark, const IBString &projection,
	   const IBString &legsStr) {}
void PosixIBClient::scannerDataEnd(int reqId) {}
void PosixIBClient::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
								   long volume, double wap, int count) {}
void PosixIBClient::fundamentalData(TickerId reqId, const IBString& data) {}
void PosixIBClient::deltaNeutralValidation(int reqId, const UnderComp& underComp) {}
void PosixIBClient::tickSnapshotEnd(int reqId) {}
void PosixIBClient::marketDataType(TickerId reqId, int marketDataType) {}
void PosixIBClient::commissionReport( const CommissionReport& commissionReport) {}
void PosixIBClient::position( const IBString& account, const Contract& contract, int position, double avgCost) {}
void PosixIBClient::positionEnd() {}
void PosixIBClient::accountSummary( int reqId, const IBString& account, const IBString& tag, const IBString& value, const IBString& curency) {}
void PosixIBClient::accountSummaryEnd( int reqId) {}
void PosixIBClient::verifyMessageAPI( const IBString& apiData) {}
void PosixIBClient::verifyCompleted( bool isSuccessful, const IBString& errorText) {}
void PosixIBClient::displayGroupList( int reqId, const IBString& groups) {}
void PosixIBClient::displayGroupUpdated( int reqId, const IBString& contractInfo) {}
