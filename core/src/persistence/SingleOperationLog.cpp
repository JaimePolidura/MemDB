#include "SingleOperationLog.h"

SingleOperationLog::SingleOperationLog(configuration_t configuration, uint32_t oplogId, logger_t loggerCons): OperationLog(configuration),
    logger(loggerCons),
    operationLogBuffer(std::make_shared<OperationLogBuffer>(configuration->get<int>(ConfigurationKeys::SERVER_THREADS) + 1)),
    intermediateOplog(std::make_shared<IntermediateOplog>(configuration, oplogId)),
    oplogIndexSegment(std::make_shared<OplogIndexSegment>(configuration, loggerCons, oplogId)),
    memdbBasePath(configuration->get(ConfigurationKeys::DATA_PATH)),
    partitionPath(memdbBasePath + "/" + std::to_string(oplogId)) {
    this->initializeFiles();
    this->operationLogBuffer->setFlushCallback([this](auto toFlush){this->intermediateOplog->addAll(toFlush);});
    this->intermediateOplog->setOnFlushingIntermediate([this](auto bytes){this->oplogIndexSegment->save(bytes);});
}

bytesDiskIterator_t SingleOperationLog::getAfterTimestamp(uint64_t after, const OperationLogOptions options) {
    OplogSegmentAfterTimestampsSearchResult desc = this->oplogIndexSegment->getByAfterTimestamp(after);
    std::vector<uint8_t> intermediate = this->intermediateOplog->getAllBytes();

    return std::make_shared<OplogIterator>(desc.descriptors, desc.descriptorInitPtr, intermediate, options.compressed, options.operationLogId,
        [this](OplogIndexSegmentDescriptor desc){return this->readBytesByIndexSegmentDescriptor(desc);});
}

bytesDiskIterator_t SingleOperationLog::getBetweenTimestamps(uint64_t fromTimestamp, uint64_t toTimestamp, const OperationLogOptions options = {}) {
    auto segmentsResult = this->oplogIndexSegment->getBetweenTimestamps(fromTimestamp, toTimestamp);

    if(segmentsResult.resultsOnlyInIntermediate) {
        return std::make_shared<OplogIterator>(std::vector<OplogIndexSegmentDescriptor>{}, 0, this->intermediateOplog->getAllBytes(), options.compressed, options.operationLogId,
            [this](OplogIndexSegmentDescriptor desc){return this->readBytesByIndexSegmentDescriptor(desc);});
    }
    if(segmentsResult.resultsInDescriptorsAndIntermediate){
        return std::make_shared<OplogIterator>(segmentsResult.descriptors, segmentsResult.descriptorInitPtr, this->intermediateOplog->getAllBytes(), options.compressed, options.operationLogId,
            [this](OplogIndexSegmentDescriptor desc){return this->readBytesByIndexSegmentDescriptor(desc);});
    }
    if(segmentsResult.resultsOnlyInSegments){
        return std::make_shared<OplogIterator>(segmentsResult.descriptors, segmentsResult.descriptorInitPtr, std::vector<uint8_t>{}, options.compressed, options.operationLogId,
            [this](OplogIndexSegmentDescriptor desc){return this->readBytesByIndexSegmentDescriptor(desc);});
    }
}

bytesDiskIterator_t SingleOperationLog::getAll(const OperationLogOptions option) {
    std::vector<OplogIndexSegmentDescriptor> desc = this->oplogIndexSegment->getAll();
    std::vector<uint8_t> intermediate = this->intermediateOplog->getAllBytes();

    return std::make_shared<OplogIterator>(desc, 0, intermediate, option.compressed, option.operationLogId,
        [this](OplogIndexSegmentDescriptor desc){return this->readBytesByIndexSegmentDescriptor(desc);});
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

void SingleOperationLog::updateCorrupted(const std::vector<uint8_t>& uncorrupted, uint32_t uncompressedSize, uint64_t ptr, const OperationLogOptions options) {
    this->oplogIndexSegment->updateCorruptedSegment(uncorrupted, uncompressedSize, ptr);
}

bool SingleOperationLog::hasOplogFile(const OperationLogOptions options) {
    return true; //Method only called by MultipleOperationLog
}

std::result<std::vector<uint8_t>> SingleOperationLog::readBytesByIndexSegmentDescriptor(OplogIndexSegmentDescriptor descriptor) {
    return this->oplogIndexSegment->getDataByDescriptorBytes(descriptor);
}

void SingleOperationLog::initializeFiles() {
    if(!FileUtils::exists(this->memdbBasePath)){
        FileUtils::createDirectory(this->memdbBasePath);
    }
    if(!FileUtils::exists(partitionPath)){
        FileUtils::createDirectory(partitionPath);
    }
}