#pragma once

#include "shared.h"
#include "persistence/OperationLogDiskWriter.h"
#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"

class OperationLogBuffer {
private:
    std::vector<OperationBody> operationBuffer;
    OperationsLogDiskWriter diskWriter;
    configuration_t configuration;
    std::mutex writeBufferLock;
    std::mutex writeDiskLock;

    uint64_t oldestTimestampAdded;
    uint64_t latestTimestampAdded;

public:
    OperationLogBuffer(configuration_t configuration): configuration(configuration) {}

    virtual ~OperationLogBuffer() = default;

    virtual void add(const OperationBody& operation) {
        writeBufferLock.lock();

        this->operationBuffer.push_back(std::move(operation));

        this->latestTimestampAdded = operation.timestamp;
        if(this->operationBuffer.size() == 1){
            this->oldestTimestampAdded = operation.timestamp;
        }

        writeBufferLock.unlock();

        if(this->operationBuffer.size() >= this->configuration->get<int>(ConfigurationKeys::MEMDB_CORE_PERSISTANCE_WRITE_EVERY)){
            this->writeOperationsToDisk();
        }
    }

    std::vector<OperationBody> get() {
        return std::vector<OperationBody>(this->operationBuffer);
    }

    uint64_t getLatestTimestampAdded() {
        return this->latestTimestampAdded;
    }

    uint64_t getOldestTimestampAdded() {
        return this->oldestTimestampAdded;
    }

    void lockWritesToDisk() {
        return this->writeDiskLock.lock();
    }

    void unlockWritesToDisk() {
        return this->writeDiskLock.unlock();
    }

private:
    void writeOperationsToDisk() {
        if(!this->writeDiskLock.try_lock())
            return;

        std::vector<OperationBody> copyBuffer;

        writeBufferLock.lock();

        this->latestTimestampAdded = 0;
        this->oldestTimestampAdded = 0;
        this->operationBuffer.swap(copyBuffer);

        writeBufferLock.unlock();

        this->diskWriter.write(copyBuffer);

        this->writeDiskLock.unlock();
    }
};

using operationLogBuffer_t = std::shared_ptr<OperationLogBuffer>;