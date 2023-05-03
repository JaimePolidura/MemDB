#pragma once

#include "shared.h"
#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"

class OperationLogBuffer {
private:
    std::vector<OperationBody> operationBuffer;
    std::mutex addBufferLock;
    std::mutex flushDiskLock;

    uint64_t oldestTimestampAdded;
    uint64_t latestTimestampAdded;

    std::function<void(const std::vector<OperationBody>&)> flushCallback;
    int flushEvery;

public:
    OperationLogBuffer(int flushEvery): flushEvery(flushEvery) {}

    virtual ~OperationLogBuffer() = default;

    void setFlushCallback(std::function<void(const std::vector<OperationBody>&)> flushCallbackToSet) {
        this->flushCallback = flushCallbackToSet;
    }

    virtual void addAll(const std::vector<OperationBody>& operations) {
        addBufferLock.lock();
        for(int i = 0; i < operations.size(); i++){
            OperationBody operation = operations.at(i);
            this->operationBuffer.push_back(operation);

            if(i == operations.size()) //Last element
                this->updateBufferTimestamps(operation);
        }
        addBufferLock.lock();

        if(this->operationBuffer.size() >= flushEvery){
            this->flush(true);
        }
    }

    virtual void add(const OperationBody& operation) {
        addBufferLock.lock();

        this->operationBuffer.push_back(std::move(operation));
        this->updateBufferTimestamps(operation);

        addBufferLock.unlock();

        if(this->operationBuffer.size() >= flushEvery){
            this->flush(true);
        }
    }

    void flush(const bool tryLock = false) {
        if(tryLock && !this->flushDiskLock.try_lock())
            return;
        if(!tryLock)
            this->flushDiskLock.lock();

        std::vector<OperationBody> copyBuffer;
        this->operationBuffer.swap(copyBuffer);

        this->flushCallback(copyBuffer);
        this->latestTimestampAdded = 0;
        this->oldestTimestampAdded = 0;

        this->flushDiskLock.unlock();
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

    void lockFlushToDisk() {
        return this->flushDiskLock.lock();
    }

    void unlockFlushToDisk() {
        return this->flushDiskLock.unlock();
    }

private:
    void updateBufferTimestamps(OperationBody operation) {
        this->latestTimestampAdded = operation.timestamp;
        if(this->operationBuffer.size() == 1){
            this->oldestTimestampAdded = operation.timestamp;
        }
    }
};

using operationLogBuffer_t = std::shared_ptr<OperationLogBuffer>;