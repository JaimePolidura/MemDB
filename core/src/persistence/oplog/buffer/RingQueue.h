#pragma once

#include "messages/request/Request.h"

#include "shared.h"

#define RING_QUEUE_INITIAL_SIZE 256

class RingQueueNode {
public:
    OperationBody operationBody;
    RingQueueNode * prev;
    bool sentinel;

    RingQueueNode(const OperationBody& operationBody, bool sentinel);
};

class RingQueue {
private:
    uint64_t lastSeen;
    RingQueueNode ** slots[RING_QUEUE_INITIAL_SIZE];

public:
    RingQueue();

    void enqueue(OperationBody& operation);

    OperationBody dequeue(int slot);
};