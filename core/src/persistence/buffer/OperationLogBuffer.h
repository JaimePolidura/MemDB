#pragma once

#include "wait-free-queues/wait-free-queues.h"
#include "OperationLogBufferFlusher.h"
#include "messages/request/Request.h"

using operationsBufferQueue_t = std::shared_ptr<jaime::unordered_mpsc_queue<OperationBody>>;

class OperationLogBuffer {
private:
    operationsBufferQueue_t buffer;
    OperationLogBufferFlusher flusher;

public:
    explicit OperationLogBuffer(int number_threads);

    virtual ~OperationLogBuffer() = default;

    void add(const OperationBody& operation);

    virtual void addAll(const std::vector<OperationBody>& operations);

    void setFlushCallback(flushCallback_t flushCallbackToSet);

    void stopFlushing();
};

using operationLogBuffer_t = std::shared_ptr<OperationLogBuffer>;