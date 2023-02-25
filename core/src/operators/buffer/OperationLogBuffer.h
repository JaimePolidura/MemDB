#pragma once

#include <vector>
#include <functional>
#include <mutex>

#include "persistence/OperationLogDiskWriter.h"
#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "utils/files/FileUtils.h"
#include "messages/request/Request.h"

class OperationLogBuffer {
private:
    std::shared_ptr<Configuration> configuration;
    std::vector<OperationBody> operationBuffer;
    OperationsLogDiskWriter diskWriter;
    std::mutex writeBufferLock;
    std::mutex writeDiskLock;

public:
    OperationLogBuffer(std::shared_ptr<Configuration> configuration): configuration(configuration) {
        this->operationBuffer.reserve(configuration->get<int>(ConfigurationKeys::PERSISTANCE_WRITE_EVERY) + 1);
    }

    void add(const OperationBody& operation) {
        writeBufferLock.lock();
        this->operationBuffer.push_back(std::move(operation));
        writeBufferLock.unlock();

        if(this->operationBuffer.size() >= this->configuration->get<int>(ConfigurationKeys::PERSISTANCE_WRITE_EVERY)){
            this->writeOperationsToDisk();
        }
    }

    std::vector<OperationBody> get() {
        return std::vector<OperationBody>(this->operationBuffer);
    }

    uint64_t getFirstTimestampWritten() {
        return this->operationBuffer.size() > 0 ? this->operationBuffer[0].timestamp : 0;
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
        this->operationBuffer.swap(copyBuffer);
        writeBufferLock.unlock();

        this->diskWriter.write(copyBuffer);

        this->writeDiskLock.unlock();
    }
};