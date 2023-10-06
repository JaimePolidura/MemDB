#include "SingleOperationLog.h"

SingleOperationLog::SingleOperationLog(configuration_t configuration, uint32_t oplogId):
    memdbBasePath(configuration->get(ConfigurationKeys::MEMDB_CORE_DATA_PATH)),
    partitionPath(memdbBasePath + "/" + std::to_string(oplogId)),
    oplogId(oplogId),
    intermediateOplog(std::make_shared<IntermediateOplog>(configuration, oplogId)) {

    this->intermediateOplog->setOnFlushingIntermediate([this](auto bytes){this->flushFromIntermediate(bytes);});
    this->initializeFiles();
}

void SingleOperationLog::add(const OperationBody &operation, const OperationLogOptions options) {
    this->intermediateOplog->add(operation);
}

void SingleOperationLog::addAll(const std::vector<OperationBody> &operations, const OperationLogOptions options) {
    this->intermediateOplog->addAll(operations);
}

uint32_t SingleOperationLog::getNumberOplogFiles() {
    return 1;
}

bool SingleOperationLog::hasOplogFile(const OperationLogOptions options) {
    return true; //Method only called by MultipleOperationLog
}

void SingleOperationLog::flushFromIntermediate(const std::vector<uint8_t>& toFlush) {


    //TODO
}

void SingleOperationLog::initializeFiles() {
    if(!FileUtils::exists(this->memdbBasePath)){
        FileUtils::createDirectory(this->memdbBasePath);
    }
    if(!FileUtils::exists(partitionPath)){
        FileUtils::createDirectory(partitionPath);
    }
}