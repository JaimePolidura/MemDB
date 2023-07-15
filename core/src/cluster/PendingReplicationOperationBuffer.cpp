#include "cluster/PendingReplicationOperationBuffer.h"

void PendingReplicationOperationBuffer::add(const Request& request){
    this->lock.lock();
    this->requests.push(request);
    this->lock.unlock();
}

bool PendingReplicationOperationBuffer::isEmpty() {
    return this->requests.empty();
}

Request PendingReplicationOperationBuffer::get() {
    this->lock.lock();

    Request toReturn = this->requests.front();
    this->requests.pop();

    this->lock.unlock();

    return toReturn;
}