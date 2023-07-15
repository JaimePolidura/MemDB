#include "persistence/oplog/SingleOperationLog.h"

SingleOperationLog::SingleOperationLog(configuration_t configuration, const std::string& fileName):
    OperationLog(configuration), operationsLogDiskWriter(fileName), operationLogDiskLoader(fileName),
    operationLogBuffer(std::make_shared<OperationLogBuffer>(configuration->get<int>(ConfigurationKeys::MEMDB_CORE_PERSISTANCE_WRITE_EVERY))) {

    this->operationLogBuffer->setFlushCallback([this](auto& operations){
        this->flushToDisk(operations);
    });
}

void SingleOperationLog::addAll(const std::vector<OperationBody>& operations, const OperationLogOptions options) {
    this->operationLogBuffer->addAll(operations);

    if(options.dontUseBuffer){
        this->operationLogBuffer->flush();
    }
}

void SingleOperationLog::add(const OperationBody& operation, const OperationLogOptions options) {
    this->operationLogBuffer->add(operation);

    if(options.dontUseBuffer){
        this->operationLogBuffer->flush();
    }
}

bool SingleOperationLog::hasOplogFile(const OperationLogOptions options) {
    return true; //Method only called by MultipleOperationLog
}

std::vector<OperationBody> SingleOperationLog::clear(const OperationLogOptions options) {
    auto operationLogsCleared = this->get(options);

    this->operationLogBuffer->flush(false);
    this->operationsLogDiskWriter.clear();

    return operationLogsCleared;
}

std::vector<OperationBody> SingleOperationLog::getAfterTimestamp(uint64_t since, const OperationLogOptions options) {
    this->operationLogBuffer->lockFlushToDisk();

    uint64_t oldestTimestampInBuffer = operationLogBuffer->getOldestTimestampAdded();
    uint64_t lastestTimestampInBuffer = operationLogBuffer->getLatestTimestampAdded();
    bool bufferEmtpy = lastestTimestampInBuffer == 0 || oldestTimestampInBuffer == 0;

    if(!bufferEmtpy && lastestTimestampInBuffer <= since){ //Already in sync
        return std::vector<OperationBody>{};
    }

    if(!bufferEmtpy && since >= oldestTimestampInBuffer && since <= lastestTimestampInBuffer) {
        std::vector<OperationBody> compactedFromBuffer = this->compacter.compact(this->operationLogBuffer->get());
        this->operationLogBuffer->unlockFlushToDisk();

        return this->filterIfTimestampAfterThan(compactedFromBuffer, since);
    }else{
        std::vector<OperationBody> compactedFromBuffer = this->operationLogBuffer->get();
        std::vector<OperationBody> compactedFromDisk = this->operationLogDiskLoader.getAll();
        this->operationLogBuffer->unlockFlushToDisk();

        std::vector<OperationBody> compacted = this->compacter.compact(Utils::concat(compactedFromDisk, compactedFromBuffer));

        return this->filterIfTimestampAfterThan(compacted, since);
    }
}

std::vector<OperationBody> SingleOperationLog::get(const OperationLogOptions option) {
    std::vector<OperationBody> fromDisk = this->operationLogDiskLoader.getAll();
    std::vector<OperationBody> compacted = this->compacter.compact(fromDisk);

    return compacted;
}

uint32_t SingleOperationLog::getNumberOplogFiles() {
    return 1;
}

std::vector<OperationBody> SingleOperationLog::filterIfTimestampAfterThan(const std::vector<OperationBody>& operations, uint64_t timestampSince) {
    std::vector<OperationBody> filtered{};

    if(operations.empty())
        return filtered;

    for(auto i = operations.end() - 1; i >= operations.begin(); i--){
        auto actualTimestamp = (* i).timestamp;

        if(actualTimestamp > timestampSince) {
            filtered.push_back(* i);
        }
    }

    return operations;
}

void SingleOperationLog::flushToDisk(const std::vector<OperationBody>& operationsInBuffer) {
    std::vector<OperationBody> compacted = this->compacter.compact(operationsInBuffer);

    this->operationsLogDiskWriter.append(compacted);
}