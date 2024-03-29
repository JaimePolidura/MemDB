#include "SingleOperationLog.h"

SingleOperationLog::SingleOperationLog(configuration_t configuration, logger_t logger): OperationLog(configuration, logger),
    operationLogBuffer(std::make_shared<OperationLogBuffer>(configuration->get<int>(ConfigurationKeys::SERVER_THREADS) + 1)),
    memdbBasePath(configuration->get(ConfigurationKeys::DATA_PATH)) {}

bytesDiskIterator_t SingleOperationLog::getAfterTimestamp(uint64_t after, const OperationLogOptions options) {
    OplogSegmentAfterTimestampsSearchResult desc = this->oplogIndexSegment->getByAfterTimestamp(after);
    std::vector<uint8_t> intermediate = this->intermediateOplog->getAllBytes();

    return std::make_shared<OplogIterator>(desc.descriptors, desc.descriptorInitPtr, intermediate, false, options.compressed, options.operationLogId,
        [this](OplogIndexSegmentDescriptor desc){return this->readBytesByIndexSegmentDescriptor(desc);});
}

bytesDiskIterator_t SingleOperationLog::getBetweenTimestamps(uint64_t fromTimestamp, uint64_t toTimestamp, const OperationLogOptions options = {}) {
    auto segmentsResult = this->oplogIndexSegment->getBetweenTimestamps(fromTimestamp, toTimestamp);

    if(segmentsResult.resultsOnlyInIntermediate) {
        return std::make_shared<OplogIterator>(std::vector<OplogIndexSegmentDescriptor>{}, 0, this->intermediateOplog->getAllBytes(), false, options.compressed, options.operationLogId,
            [this](OplogIndexSegmentDescriptor desc){return this->readBytesByIndexSegmentDescriptor(desc);});
    }
    if(segmentsResult.resultsInDescriptorsAndIntermediate) {
        return std::make_shared<OplogIterator>(segmentsResult.descriptors, segmentsResult.descriptorInitPtr, this->intermediateOplog->getAllBytes(), false, options.compressed, options.operationLogId,
            [this](OplogIndexSegmentDescriptor desc){return this->readBytesByIndexSegmentDescriptor(desc);});
    }
    if(segmentsResult.resultsOnlyInSegments) {
        return std::make_shared<OplogIterator>(segmentsResult.descriptors, segmentsResult.descriptorInitPtr, std::vector<uint8_t>{}, true, options.compressed, options.operationLogId,
            [this](OplogIndexSegmentDescriptor desc){return this->readBytesByIndexSegmentDescriptor(desc);});
    }

    throw std::error("SegmentResult in SingleOperationLog::getBetweenTimestamps cannot be false");
}

bytesDiskIterator_t SingleOperationLog::getAll(const OperationLogOptions option) {
    std::vector<OplogIndexSegmentDescriptor> desc = this->oplogIndexSegment->getAll();
    std::vector<uint8_t> intermediate = this->intermediateOplog->getAllBytes();

    return std::make_shared<OplogIterator>(desc, 0, intermediate, false, option.compressed, option.operationLogId,
        [this](OplogIndexSegmentDescriptor desc){return this->readBytesByIndexSegmentDescriptor(desc);});
}

void SingleOperationLog::clear(memdbOplogId_t oplogId) {
    this->operationLogBuffer->stopFlushing();
    this->oplogIndexSegment->clearAll();
    this->intermediateOplog->clearAll();
}

void SingleOperationLog::add(memdbOplogId_t oplogId, const OperationBody &operation) {
    this->operationLogBuffer->add(operation);
}

void SingleOperationLog::addAll(memdbOplogId_t oplogId, const std::vector<OperationBody> &operations) {
    this->operationLogBuffer->addAll(operations);
}

uint32_t SingleOperationLog::getNumberOplogFiles() {
    return 1;
}

void SingleOperationLog::updateCorrupted(const std::vector<uint8_t>& uncorrupted, uint32_t uncompressedSize, uint64_t ptr, memdbOplogId_t oplogId) {
    this->logger->debugInfo("Fixed {0} bytes corrupted oplog segment in oplogId {1} at desc ptr {2}", uncorrupted.size(), oplogId, ptr);

    this->oplogIndexSegment->updateCorruptedSegment(uncorrupted, uncompressedSize, ptr);
}

bool SingleOperationLog::hasOplogFile(memdbOplogId_t oplogId) {
    return true; //Method only called by MultipleOperationLog
}

std::result<std::vector<uint8_t>> SingleOperationLog::readBytesByIndexSegmentDescriptor(OplogIndexSegmentDescriptor descriptor) {
    return this->oplogIndexSegment->getDataByDescriptorBytes(descriptor);
}

void SingleOperationLog::initialize(uint32_t oplogId) {
    this->oplogIndexSegment = std::make_shared<OplogIndexSegment>(configuration, logger, oplogId);
    this->intermediateOplog = std::make_shared<IntermediateOplog>(configuration, oplogId);
    this->partitionPath = this->memdbBasePath + "/" + std::to_string(oplogId);

    this->initializeFiles();
    this->operationLogBuffer->setFlushCallback([this](auto toFlush){this->intermediateOplog->addAll(toFlush);});
    this->intermediateOplog->setOnFlushingIntermediate([this](auto bytes){this->oplogIndexSegment->save(bytes);});
}

void SingleOperationLog::initializeFiles() {
    if(!FileUtils::exists(this->memdbBasePath)){
        FileUtils::createDirectory(this->memdbBasePath);
    }
    if(!FileUtils::exists(partitionPath)){
        FileUtils::createDirectory(partitionPath);
    }
}