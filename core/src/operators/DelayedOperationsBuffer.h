#pragma once

#include "messages/request/Request.h"
#include "shared.h"

//TODO Add disk backing
class DelayedOperationsBuffer {
private:
    std::queue<Request> operations;
    std::mutex lock;

public:
    void add(const Request& request){
        this->lock.lock();
        this->operations.push(request);
        this->lock.unlock();
    }

    bool isEmpty() {
        return this->operations.empty();
    }

    Request get() {
        this->lock.lock();

        Request toReturn = this->operations.front();
        this->operations.pop();

        this->lock.unlock();

        return toReturn;
    }
};

using delayedOperationsBuffer_t = std::shared_ptr<DelayedOperationsBuffer>;