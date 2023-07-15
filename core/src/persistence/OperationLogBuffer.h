#pragma once

#include "shared.h"
#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "messages/request/Request.h"

class OperationLogBuffer {
private:
    std::vector<OperationBody> operationBuffer;
    std::mutex addBufferLock;
    std::mutex flushDiskLock;

    uint64_t oldestTimestampAdded{};
    uint64_t latestTimestampAdded{};

    std::function<void(const std::vector<OperationBody>&)> flushCallback;
    int flushEvery;

public:
    explicit OperationLogBuffer(int flushEvery): flushEvery(flushEvery) {}

    virtual ~OperationLogBuffer() = default;

    void setFlushCallback(std::function<void(const std::vector<OperationBody>&)> flushCallbackToSet);

    virtual void addAll(const std::vector<OperationBody>& operations);

    virtual void add(const OperationBody& operation);

    void flush(const bool tryLock = false);

    std::vector<OperationBody> get();

    uint64_t getLatestTimestampAdded() const;

    uint64_t getOldestTimestampAdded() const;

    void lockFlushToDisk();

    void unlockFlushToDisk();

private:
    void updateBufferTimestamps(const OperationBody& operation);
};

using operationLogBuffer_t = std::shared_ptr<OperationLogBuffer>;