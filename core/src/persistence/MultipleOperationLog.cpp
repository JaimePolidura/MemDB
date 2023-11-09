#include "MultipleOperationLog.h"

MultipleOperationLog::MultipleOperationLog(configuration_t configuration, logger_t logger, std::function<std::string(int)> oplogFileNameResolver):
    OperationLog(configuration, logger), oplogFileNameResolver(oplogFileNameResolver) {}

void MultipleOperationLog::addAll(memdbOplogId_t oplogId, const std::vector<OperationBody>& operations) {
    for (const OperationBody& operation: operations) {
        this->add(oplogId, operation);
    }
}

void MultipleOperationLog::add(memdbOplogId_t oplogId, const OperationBody& operation) {
    singleOperationLog_t oplog = this->operationLogs[oplogId];

    oplog->add(oplogId, operation);
}

bool MultipleOperationLog::hasOplogFile(memdbOplogId_t oplogId) {
    return this->operationLogs.size() < oplogId &&
            this->operationLogs.at(oplogId)->hasOplogFile(oplogId);
}

void MultipleOperationLog::clear(memdbOplogId_t oplogId) {
    if(oplogId >= this->operationLogs.size()){
        return;
    }

    this->operationLogs.at(oplogId)->clear(oplogId);
}

void MultipleOperationLog::updateCorrupted(const std::vector<uint8_t>& uncorrupted, uint32_t uncompressedSize, uint64_t ptr, memdbOplogId_t oplogId) {
    if(oplogId >= this->operationLogs.size()){
        return;
    }

    return this->operationLogs[oplogId]->updateCorrupted(uncorrupted, uncompressedSize, ptr, oplogId);
}

bytesDiskIterator_t MultipleOperationLog::getBetweenTimestamps(uint64_t fromTimestamp, uint64_t toTimestamp, const OperationLogOptions options) {
    if(options.operationLogId >= this->operationLogs.size()){
        return std::make_shared<NullIterator<std::result<std::vector<uint8_t>>>>();
    }

    return this->operationLogs[options.operationLogId]->getBetweenTimestamps(fromTimestamp, toTimestamp, options);
}

bytesDiskIterator_t MultipleOperationLog::getAfterTimestamp(uint64_t timestamp, OperationLogOptions options) {
    if(options.operationLogId >= this->operationLogs.size()){
        return std::make_shared<NullIterator<std::result<std::vector<uint8_t>>>>();
    }

    return this->operationLogs[options.operationLogId]->getAfterTimestamp(timestamp, options);
}

bytesDiskIterator_t MultipleOperationLog::getAll(const OperationLogOptions options) {
    if(options.operationLogId >= this->operationLogs.size()){
        return std::make_shared<NullIterator<std::result<std::vector<uint8_t>>>>();
    }

    return this->operationLogs[options.operationLogId]->getAll(options);
}

uint32_t MultipleOperationLog::getNumberOplogFiles() {
    return this->operationLogs.size();
}

void MultipleOperationLog::initializeOplogs(int numberOplogs) {
    for(int i = 0; i < numberOplogs; i++) {
        std::string fileName = oplogFileNameResolver(i);
        singleOperationLog_t oplog = std::make_shared<SingleOperationLog>(this->configuration, this->logger);

        this->operationLogs.push_back(oplog);
    }
}