#pragma once

#include "shared.h"

#include "messages/request/Request.h"
#include "persistence/compaction/OperationLogCompacter.h"
#include "persistence/OperationLogDiskLoader.h"
#include "persistence/OperationLogBuffer.h"

#include "config/Configuration.h"
#include "utils/Utils.h"
#include "OperationLogDiskWriter.h"

class OperationLog {
private:
    OperationsLogDiskWriter operationsLogDiskWriter;
    OperationLogDiskLoader operationLogDiskLoader;
    OperationLogCompacter compacter;

    operationLogBuffer_t operationLogBuffer;
    configuration_t configuration;

public:
    OperationLog(configuration_t configuration): configuration(configuration), operationsLogDiskWriter("oplog"), operationLogDiskLoader("oplog"),
        operationLogBuffer(std::make_shared<OperationLogBuffer>(configuration->get<int>(ConfigurationKeys::MEMDB_CORE_PERSISTANCE_WRITE_EVERY))) {

        this->operationLogBuffer->setFlushCallback([this](auto& operations){
            this->flushToDisk(operations);
        });
    }

    void add(const OperationBody& operation) {
        this->operationLogBuffer->add(operation);
    }

    std::vector<OperationBody> getAllAfterTimestamp(uint64_t since) {
        this->lockWritesToDisk();

        uint64_t oldestTimestampInBuffer = operationLogBuffer->getOldestTimestampAdded();
        uint64_t lastestTimestampInBuffer = operationLogBuffer->getLatestTimestampAdded();
        bool bufferEmtpy = lastestTimestampInBuffer == 0 || oldestTimestampInBuffer == 0;

        if(!bufferEmtpy && lastestTimestampInBuffer <= since){ //Already in sync
            return std::vector<OperationBody>{};
        }

        if(!bufferEmtpy && since >= oldestTimestampInBuffer && since <= lastestTimestampInBuffer) {
            std::vector<OperationBody> compactedFromBuffer = this->compacter.compact(this->operationLogBuffer->get());
            this->unlockWritesToDisk();

            return this->fiterIfTimestampAfterThan(compactedFromBuffer, since);
        }else{
            std::vector<OperationBody> compactedFromBuffer = this->operationLogBuffer->get();
            std::vector<OperationBody> compactedFromDisk = this->operationLogDiskLoader.getAll();
            this->unlockWritesToDisk();

            std::vector<OperationBody> compacted = this->compacter.compact(Utils::concat(compactedFromDisk, compactedFromBuffer));

            return this->fiterIfTimestampAfterThan(compacted, since);
        }
    }

    std::vector<OperationBody> getFromDisk() {
        std::vector<OperationBody> fromDisk = this->operationLogDiskLoader.getAll();
        std::vector<OperationBody> compacted = this->compacter.compact(fromDisk);
        this->operationsLogDiskWriter.write(compacted);

        return compacted;
    }

private:
    std::vector<OperationBody> fiterIfTimestampAfterThan(const std::vector<OperationBody>& operations, uint64_t timestampSince) {
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

    void lockWritesToDisk() {
        return this->operationLogBuffer->lockFlushToDisk();
    }

    void unlockWritesToDisk() {
        return this->operationLogBuffer->unlockFlushToDisk();
    }

    void flushToDisk(const std::vector<OperationBody>& operationsInBuffer) {
        std::vector<OperationBody> compacted = this->compacter.compact(operationsInBuffer);

        this->operationsLogDiskWriter.write(compacted);
    }
};

using operationLog_t = std::shared_ptr<OperationLog>;