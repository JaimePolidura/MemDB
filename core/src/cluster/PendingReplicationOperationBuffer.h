#pragma once

#include "messages/request/Request.h"
#include "shared.h"

class PendingReplicationOperationBuffer {
private:
    std::queue<Request> requests;
    std::mutex lock;

public:
    void add(const Request& request);

    bool isEmpty();

    Request get();
};

using replicationOperationBuffer_t = std::shared_ptr<PendingReplicationOperationBuffer>;