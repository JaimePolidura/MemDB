#include "OperationLogBufferFlusher.h"

OperationLogBufferFlusher::OperationLogBufferFlusher(operationsBufferQueue_t buffer): buffer(buffer) {}

void OperationLogBufferFlusher::startFlushing(flushCallback_t flushCallback) {
    this->flusherThread = std::thread([this, flushCallback]() {
        while(!this->stopFlushingFlag.load(std::memory_order_acquire)) {
            std::vector<OperationBody> operations = this->buffer->dequeue_all_or_sleep_for(std::chrono::milliseconds(100));
            flushCallback(operations);
        }

        this->flusherExited.store(true, std::memory_order_release);
        this->flusherExitCondVariable.notify_all();
    });
}

void OperationLogBufferFlusher::stopFlushing() {
    this->stopFlushingFlag.store(true, std::memory_order_release);

    std::atomic_thread_fence(std::memory_order_acquire);

    std::unique_lock<std::mutex> lock(flusherExitMutex);
    this->flusherExitCondVariable.wait(lock, [this] { return this->flusherExited.load(std::memory_order_acquire); });
}