#include "SingleOperationLog.h"

SingleOperationLog::SingleOperationLog(configuration_t configuration, uint32_t oplogId): OperationLog(configuration),
    operationLogBuffer(std::make_shared<OperationLogBuffer>(configuration->get<int>(ConfigurationKeys::MEMDB_CORE_SERVER_THREADS) + 1)),
    intermediateOplog(std::make_shared<IntermediateOplog>(configuration, oplogId)),
    memdbBasePath(configuration->get(ConfigurationKeys::MEMDB_CORE_DATA_PATH)),
    partitionPath(memdbBasePath + "/" + std::to_string(oplogId)) {
    this->operationLogBuffer->setFlushCallback([this](auto toFlush){this->intermediateOplog->addAll(toFlush);});
    this->intermediateOplog->setOnFlushingIntermediate([this](auto bytes){this->oplogIndexSegment_t->save(bytes);;});
    this->initializeFiles();
}

std::vector<OperationBody> SingleOperationLog::clear(const OperationLogOptions options) {}
std::vector<OperationBody> SingleOperationLog::getAfterTimestamp(uint64_t since, const OperationLogOptions options) {}
std::vector<OperationBody> SingleOperationLog::get(const OperationLogOptions option) {}

void SingleOperationLog::add(const OperationBody &operation, const OperationLogOptions options) {
    this->operationLogBuffer->add(operation);
}

void SingleOperationLog::addAll(const std::vector<OperationBody> &operations, const OperationLogOptions options) {
    this->operationLogBuffer->addAll(operations);
}

uint32_t SingleOperationLog::getNumberOplogFiles() {
    return 1;
}

bool SingleOperationLog::hasOplogFile(const OperationLogOptions options) {
    return true; //Method only called by MultipleOperationLog
}

void SingleOperationLog::initializeFiles() {
    if(!FileUtils::exists(this->memdbBasePath)){
        FileUtils::createDirectory(this->memdbBasePath);
    }
    if(!FileUtils::exists(partitionPath)){
        FileUtils::createDirectory(partitionPath);
    }
}