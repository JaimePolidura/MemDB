#include "persistence/OperationLogBuffer.h"

#include <utility>

void OperationLogBuffer::setFlushCallback(std::function<void(const std::vector<OperationBody>&)> flushCallbackToSet) {
    this->flushCallback = std::move(flushCallbackToSet);
}

void OperationLogBuffer::addAll(const std::vector<OperationBody>& operations) {
    addBufferLock.lock();
    for(int i = 0; i < operations.size(); i++){
        const OperationBody& operation = operations.at(i);
        this->operationBuffer.push_back(operation);

        if(i == operations.size()) //Last element
            this->updateBufferTimestamps(operation);
    }
    addBufferLock.lock();

    if(this->operationBuffer.size() >= flushEvery){
        this->flush(true);
    }
}

void OperationLogBuffer::add(const OperationBody& operation) {
    addBufferLock.lock();

    this->operationBuffer.push_back(operation);
    this->updateBufferTimestamps(operation);

    addBufferLock.unlock();

    if(this->operationBuffer.size() >= flushEvery){
        this->flush(true);
    }
}

void OperationLogBuffer::flush(const bool tryLock) {
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

std::vector<OperationBody> OperationLogBuffer::get() {
    return std::vector<OperationBody>(this->operationBuffer);
}

uint64_t OperationLogBuffer::getLatestTimestampAdded() const {
    return this->latestTimestampAdded;
}

uint64_t OperationLogBuffer::getOldestTimestampAdded() const {
    return this->oldestTimestampAdded;
}

void OperationLogBuffer::lockFlushToDisk() {
    return this->flushDiskLock.lock();
}

void OperationLogBuffer::unlockFlushToDisk() {
    return this->flushDiskLock.unlock();
}

void OperationLogBuffer::updateBufferTimestamps(const OperationBody& operation) {
    this->latestTimestampAdded = operation.timestamp;
    if(this->operationBuffer.size() == 1){
        this->oldestTimestampAdded = operation.timestamp;
    }
}