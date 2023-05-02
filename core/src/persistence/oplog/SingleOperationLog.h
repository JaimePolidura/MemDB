#pragma once

#include "shared.h"

#include "messages/request/Request.h"

#include "persistence/compaction/OperationLogCompacter.h"
#include "persistence/OperationLogDiskLoader.h"
#include "persistence/OperationLogBuffer.h"
#include "persistence/OperationLogDiskWriter.h"
#include "persistence/oplog/OperationLog.h"

#include "config/Configuration.h"
#include "utils/Utils.h"

class SingleOperationLog : public OperationLog {
private:
    OperationsLogDiskWriter operationsLogDiskWriter;
    OperationLogDiskLoader operationLogDiskLoader;
    OperationLogCompacter compacter;

    operationLogBuffer_t operationLogBuffer;

public:
    SingleOperationLog(configuration_t configuration, const std::string& fileName):
        OperationLog(configuration), operationsLogDiskWriter(fileName), operationLogDiskLoader(fileName),
        operationLogBuffer(std::make_shared<OperationLogBuffer>(configuration->get<int>(ConfigurationKeys::MEMDB_CORE_PERSISTANCE_WRITE_EVERY))) {

        this->operationLogBuffer->setFlushCallback([this](auto& operations){
            this->flushToDisk(operations);
        });
    }

    void add(const OperationBody& operation) override {
        this->operationLogBuffer->add(operation);
    }

    void replaceAll(const std::vector<OperationBody>& toReplace, const OperationLogOptions options = {}) override {
        this->operationsLogDiskWriter.write(toReplace);
    }

    std::vector<OperationBody> getAfterTimestamp(uint64_t since, const OperationLogOptions options = {}) override {
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

            return this->fiterIfTimestampAfterThan(compactedFromBuffer, since);
        }else{
            std::vector<OperationBody> compactedFromBuffer = this->operationLogBuffer->get();
            std::vector<OperationBody> compactedFromDisk = this->operationLogDiskLoader.getAll();
            this->operationLogBuffer->unlockFlushToDisk();

            std::vector<OperationBody> compacted = this->compacter.compact(Utils::concat(compactedFromDisk, compactedFromBuffer));

            return this->fiterIfTimestampAfterThan(compacted, since);
        }
    }

    std::vector<OperationBody> getAllFromDisk(const OperationLogOptions options = {}) override {
        std::vector<OperationBody> fromDisk = this->operationLogDiskLoader.getAll();
        std::vector<OperationBody> compacted = this->compacter.compact(fromDisk);

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

    void flushToDisk(const std::vector<OperationBody>& operationsInBuffer) {
        std::vector<OperationBody> compacted = this->compacter.compact(operationsInBuffer);

        this->operationsLogDiskWriter.append(compacted);
    }
};

using singleOperationLog_t = std::shared_ptr<SingleOperationLog>;