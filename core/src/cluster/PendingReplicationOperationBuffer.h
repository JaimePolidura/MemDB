#pragma once

#include "messages/request/Request.h"
#include "shared.h"

class PendingReplicationOperationBuffer {
private:
    std::queue<Request> requests;
    std::mutex lock;

public:
    void add(const Request& request){
        this->lock.lock();
        this->requests.push(request);
        this->lock.unlock();
    }

    bool isEmpty() {
        return this->requests.empty();
    }

    Request get() {
        this->lock.lock();

        Request toReturn = this->requests.front();
        this->requests.pop();

        this->lock.unlock();

        return toReturn;
    }
};

using replicationOperationBuffer_t = std::shared_ptr<PendingReplicationOperationBuffer>;