/* Copyright (C) 2013 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */
#include <stdio.h>
#include <QDebug>
#include "PosixIBClient.h"

#include "EPosixClientSocket.h"
#include "EPosixClientSocketPlatform.h"

#include "Contract.h"
#include "Order.h"



///////////////////////////////////////////////////////////
// member funcs
PosixIBClient::PosixIBClient(QObject *parent) : QObject(parent) //QObject *parent   : QObject(parent)
{

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

void PosixIBClient::processMessages()
{
    qDebug() <<"starting processMessages() infinite loop";
    while(isConnected())
    {
        fd_set readSet, writeSet, errorSet;

        struct timeval tval;
        tval.tv_usec = 0;
        tval.tv_sec = 0;

        if( pClient->fd() >= 0 )
        {


            FD_ZERO( &readSet);
            errorSet = writeSet = readSet;

            FD_SET( pClient->fd(), &readSet);

            if( !pClient->isOutBufferEmpty())
                FD_SET( pClient->fd(), &writeSet);

            FD_SET( pClient->fd(), &errorSet);

            int ret = select( pClient->fd() + 1, &readSet, &writeSet, &errorSet, &tval);

//            if( ret == 0) { // timeout
//                qDebug() <<"looping..ret = 0 timeout return";
//                return;
//            }

            if( ret < 0) {	// error
                qDebug() <<"looping..ret < 0 , disconn, error return";
                disconnect();

                return;
            }

            if( pClient->fd() < 0)
                return;

            if( FD_ISSET( pClient->fd(), &errorSet)) {
                // error on socket
                qDebug() <<"looping..errorSet triggered";
                pClient->onError();
            }

            if( pClient->fd() < 0)
            {
                qDebug() <<"looping..fd() < 0 , return b4 writeset";
                return;
            }

            if( FD_ISSET( pClient->fd(), &writeSet)) {
                // socket is ready for writing
                pClient->onSend();
            }

            if( pClient->fd() < 0)
            {
                qDebug() <<"looping..fd() < 0 , return b4 readset";
                return;
            }

            if( FD_ISSET( pClient->fd(), &readSet)) {
                // socket is ready for reading
                pClient->onReceive();
            }


        }

    }

}

void PosixIBClient::onTest()
{

    pClient->reqIds(nextID);
    qDebug() << "Placing order ....";
    placeOrder();

//    qDebug() << "Requesting market data ....";
//    onReqMktData();
}

void PosixIBClient::sayHello()
{
    qDebug() <<"Hello from PosixIBClient";
    qDebug() << QThread::currentThreadId();
}

void PosixIBClient::onConnect(int port,  int pubport,  int subport)
{
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

        processMessages();

    }
    else
    {

        qDebug() << "Failed to connect. Max no of try reached.";
        emit disconnected();
    }
}

void PosixIBClient::onDisconnect()
{

    disconnect();

    while(isConnected())
    {
        sleep(2);
    }

    qDebug() <<"Disonnected";
    emit disconnected();
}

void PosixIBClient::onReqCurrentTime()
{

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



int PosixIBClient::getNextValidID()
{
    int useID;
    useID = nextID;
    nextID += 1;
    return useID;
}

bool PosixIBClient::connect(const char *host, unsigned int port, int clientId)
{
	// trying to connect
    bool bRes = pClient->eConnect( host, port, clientId, /* extraAuth */ false);
	return bRes;
}

void PosixIBClient::disconnect() const
{
    pClient->eDisconnect();

}

bool PosixIBClient::isConnected() const
{
    return pClient->isConnected();
}


//////////////////////////////////////////////////////////////////
// methods
void PosixIBClient::reqCurrentTime()
{
    pClient->reqCurrentTime();
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

    order.action = "SELL";
    order.totalQuantity = 1;
	order.orderType = "LMT";
    order.lmtPrice = 2018.5;

    qDebug() << "VALID ID:" << nextID;
    pClient->placeOrder( getNextValidID(), contract, order);
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
    qDebug() << "OrderStatus:" << QString::fromStdString(status);
}

void PosixIBClient::nextValidId( OrderId orderId)
{
    qDebug() << "nextValidId:" << orderId;
    nextID = orderId;
}

void PosixIBClient::currentTime( long time)
{

}

void PosixIBClient::error(const int id, const int errorCode, const IBString errorString)
{

	if( id == -1 && errorCode == 1100) // if "Connectivity between IB and TWS has been lost"
		disconnect();
}

void PosixIBClient::tickPrice( TickerId tickerId, TickType field, double price, int canAutoExecute)
{
    qDebug() << "Tick price";

}
void PosixIBClient::tickSize( TickerId tickerId, TickType field, int size)
{
    qDebug() << "Tick size";

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