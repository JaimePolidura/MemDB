#pragma once

#include "messages/request/Request.h"
#include "shared.h"

//TODO Add disk backing
class DelayedOperationsBuffer {
private:
    std::queue<Request> operations;
    std::mutex lock;

public:
    void add(const Request& request);

    bool isEmpty();

    Request get();
};

using delayedOperationsBuffer_t = std::shared_ptr<DelayedOperationsBuffer>;