#pragma once

#include "wait-free-queues/wait-free-queues.h"
#include "messages/request/Request.h"

using operationsBufferQueue_t = std::shared_ptr<jaime::unordered_mpsc_queue<OperationBody>>;
using flushCallback_t = std::function<void(const std::vector<OperationBody>&)>;

class OperationLogBufferFlusher {
private:
    operationsBufferQueue_t buffer;
    std::thread flusherThread;

    std::atomic_bool stopFlushingFlag{false};
    std::atomic_bool flusherExited{false};
    std::mutex flusherExitMutex{};
    std::condition_variable flusherExitCondVariable;

public:
    explicit OperationLogBufferFlusher(operationsBufferQueue_t buffer);

    void startFlushing(flushCallback_t flushCallback);

    void stopFlushing();
};
