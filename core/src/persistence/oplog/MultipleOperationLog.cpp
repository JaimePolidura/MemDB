#include "persistence/oplog/MultipleOperationLog.h"

MultipleOperationLog::MultipleOperationLog(configuration_t configuration, std::function<int(const OperationBody&)> oplogResolver,
        std::function<std::string(int)> oplogFileNameResolver, uint32_t numberOplogs): OperationLog(configuration), oplogResolver(oplogResolver) {
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

std::vector<OperationBody> MultipleOperationLog::clear(const OperationLogOptions options) {
    if(options.operationLogId >= this->operationLogs.size()){
        return std::vector<OperationBody>{};
    }

    singleOperationLog_t operationLogToClear = this->operationLogs.at(options.operationLogId);
    std::vector<OperationBody> operationsCleared = operationLogToClear->clear(options);

    this->operationLogs.erase(this->operationLogs.begin() + options.operationLogId);

    return operationsCleared;
}

std::vector<OperationBody> MultipleOperationLog::getAfterTimestamp(uint64_t timestamp, OperationLogOptions options) {
    singleOperationLog_t oplog = this->operationLogs[options.operationLogId];

    return oplog->getAfterTimestamp(timestamp, options);
}

std::vector<OperationBody> MultipleOperationLog::get(const OperationLogOptions options) {
    std::vector<OperationBody> totalFromDisk{};

    return this->operationLogs.at(options.operationLogId)->get(options);
}

uint32_t MultipleOperationLog::getNumberOplogFiles() {
    return this->operationLogs.size();
}

void MultipleOperationLog::flush() {
    for (const auto &operationLog: this->operationLogs) {
        operationLog->flush();
    }
}

void MultipleOperationLog::initializeOplogs(int numberOplogs, std::function<std::string(int)> oplogFileNameResolver) {
    for(int i = 0; i < numberOplogs; i++) {
        std::string fileName = oplogFileNameResolver(i);
        singleOperationLog_t oplog = std::make_shared<SingleOperationLog>(this->configuration, fileName);

        this->operationLogs.push_back(oplog);
    }
}