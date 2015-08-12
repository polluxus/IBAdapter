#include "messageprocessor.h"

MessageProcessor::MessageProcessor(std::shared_ptr<PosixIBClient> pIBClient, QObject *parent)
     : QThread(parent), pIBClient(pIBClient){
 }

MessageProcessor::~MessageProcessor(){

}
