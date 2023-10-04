#include "OperationLogBufferFlusher.h"

OperationLogBufferFlusher::OperationLogBufferFlusher(operationsBufferQueue_t buffer): buffer(buffer) {}

void OperationLogBufferFlusher::startFlushing(flushCallback_t flushCallback) {
    this->flusherThread = std::thread([this, flushCallback]() {
        while(true) {
            std::vector<OperationBody> operations = this->buffer->dequeue_all_or_sleep_for(std::chrono::milliseconds(100));
            flushCallback(operations);
        }
    });
}