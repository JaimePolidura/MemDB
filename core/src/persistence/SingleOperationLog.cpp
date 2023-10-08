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

OplogSegmentIterator SingleOperationLog::getAfterTimestamp(uint64_t after, const OperationLogOptions options) {
    std::vector<OplogIndexSegmentDescriptor> desc = this->oplogIndexSegment_t->getByAfterTimestamp(after);
    std::vector<OperationBody> intermediate = this->intermediateOplog->getAll();

    return OplogSegmentIterator{desc, intermediate, [this](OplogIndexSegmentDescriptor desc){return this->oplogIndexSegment_t->getDataByDescriptor(desc);}};
}

OplogSegmentIterator SingleOperationLog::get(const OperationLogOptions option) {
    std::vector<OplogIndexSegmentDescriptor> desc = this->oplogIndexSegment_t->getAll();
    std::vector<OperationBody> intermediate = this->intermediateOplog->getAll();

    return OplogSegmentIterator{desc, intermediate, [this](OplogIndexSegmentDescriptor desc){return this->oplogIndexSegment_t->getDataByDescriptor(desc);}};
}

std::vector<OperationBody> SingleOperationLog::clear(const OperationLogOptions options) {}

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