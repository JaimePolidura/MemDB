#include "SingleOperationLog.h"

SingleOperationLog::SingleOperationLog(configuration_t configuration, uint32_t oplogId): OperationLog(configuration),
    operationLogBuffer(std::make_shared<OperationLogBuffer>(configuration->get<int>(ConfigurationKeys::SERVER_THREADS) + 1)),
    intermediateOplog(std::make_shared<IntermediateOplog>(configuration, oplogId)),
    oplogIndexSegment(std::make_shared<OplogIndexSegment>(configuration, oplogId)),
    memdbBasePath(configuration->get(ConfigurationKeys::DATA_PATH)),
    partitionPath(memdbBasePath + "/" + std::to_string(oplogId)) {
    this->initializeFiles();
    this->operationLogBuffer->setFlushCallback([this](auto toFlush){this->intermediateOplog->addAll(toFlush);});
    this->intermediateOplog->setOnFlushingIntermediate([this](auto bytes){this->oplogIndexSegment->save(bytes);});
}

oplogSegmentIterator_t SingleOperationLog::getAfterTimestamp(uint64_t after, const OperationLogOptions options) {
    std::vector<OplogIndexSegmentDescriptor> desc = this->oplogIndexSegment->getByAfterTimestamp(after);
    std::vector<uint8_t> intermediate = this->intermediateOplog->getAllBytes();

    return std::make_shared<OplogIterator>(desc, intermediate, [this](OplogIndexSegmentDescriptor desc){return this->oplogIndexSegment->getDataByDescriptorBytes(desc);});
}

oplogSegmentIterator_t SingleOperationLog::getAll(const OperationLogOptions option) {
    std::vector<OplogIndexSegmentDescriptor> desc = this->oplogIndexSegment->getAll();
    std::vector<uint8_t> intermediate = this->intermediateOplog->getAllBytes();

    return std::make_shared<OplogIterator>(desc, intermediate, [this](OplogIndexSegmentDescriptor desc){return this->oplogIndexSegment->getDataByDescriptorBytes(desc);});
}

void SingleOperationLog::clear(const OperationLogOptions options) {
    this->operationLogBuffer->stopFlushing();
    this->oplogIndexSegment->clearAll();
    this->intermediateOplog->clearAll();
}

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