#include "OperationLogBuffer.h"

OperationLogBuffer::OperationLogBuffer(int number_threads):
    buffer(std::make_shared<jaime::lock_free::ordered_mpsc_queue<OperationBody>>(number_threads)),
    flusher(buffer) {}

void OperationLogBuffer::add(const OperationBody &operation) {
    this->buffer->enqueue(operation);
}

void OperationLogBuffer::addAll(const std::vector<OperationBody> &operations) {
    std::for_each(operations.begin(), operations.end(), [this](const OperationBody& it){this->buffer->enqueue(it);});
}

void OperationLogBuffer::setFlushCallback(flushCallback_t flushCallbackToSet) {
    this->flusher.startFlushing(flushCallbackToSet);
}