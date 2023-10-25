#pragma once

#include "messages/request/Request.h"
#include "shared.h"

//TODO Add disk backing
class DelayedOperationsBuffer {
private:
    std::queue<Request> operations{};
    std::mutex lock;

public:
    DelayedOperationsBuffer() = default;

    void add(const Request& request);

    bool isEmpty();

    Request get();
};
