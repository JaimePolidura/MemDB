#include "operators/DelayedOperationsBuffer.h"

void DelayedOperationsBuffer::add(const Request& request){
    this->lock.lock();
    this->operations.push(request);
    this->lock.unlock();
}

bool DelayedOperationsBuffer::isEmpty() {
    return this->operations.empty();
}

Request DelayedOperationsBuffer::get() {
    this->lock.lock();

    Request toReturn = this->operations.front();
    this->operations.pop();

    this->lock.unlock();

    return toReturn;
}