#include "MultipleOperationLog.h"

MultipleOperationLog::MultipleOperationLog(configuration_t configuration, std::function<int(const OperationBody&)> oplogResolver,
        std::function<std::string(int)> oplogFileNameResolver, uint32_t numberOplogs, logger_t loggerCons): OperationLog(configuration),
        oplogResolver(oplogResolver),
        logger(loggerCons) {
    this->initializeOplogs(numberOplogs, oplogFileNameResolver);
}

void MultipleOperationLog::addAll(const std::vector<OperationBody>& operations, const OperationLogOptions options) {
    for (const OperationBody& operation: operations) {
        this->add(operation, options);
    }
}

void MultipleOperationLog::add(const OperationBody& operation, const OperationLogOptions options) {
    int oplogId = this->oplogResolver(operation);
    singleOperationLog_t oplog = this->operationLogs[oplogId];

    oplog->add(operation, options);
}

bool MultipleOperationLog::hasOplogFile(const OperationLogOptions options) {
    return this->operationLogs.size() < options.operationLogId &&
            this->operationLogs.at(options.operationLogId)->hasOplogFile(options);
}

void MultipleOperationLog::clear(const OperationLogOptions options) {
    if(options.operationLogId >= this->operationLogs.size()){
        return;
    }

    this->operationLogs.at(options.operationLogId)->clear(options);
}

void MultipleOperationLog::updateCorrupted(const std::vector<uint8_t>& uncorrupted, uint32_t uncompressedSize, uint64_t ptr, const OperationLogOptions options) {
    if(options.operationLogId >= this->operationLogs.size()){
        return;
    }

    return this->operationLogs[options.operationLogId]->updateCorrupted(uncorrupted, uncompressedSize, ptr, options);
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

void MultipleOperationLog::initializeOplogs(int numberOplogs, std::function<std::string(int)> oplogFileNameResolver) {
    for(int i = 0; i < numberOplogs; i++) {
        std::string fileName = oplogFileNameResolver(i);
        singleOperationLog_t oplog = std::make_shared<SingleOperationLog>(this->configuration, i, this->logger);

        this->operationLogs.push_back(oplog);
    }
}