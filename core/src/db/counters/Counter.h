#pragma once

#include "memdbtypes.h"
#include "shared.h"

class Counter {
public:
    Counter(memdbNodeId_t selfNodeId, uint32_t nNodes);

    void increment();

    void decrement();

    uint64_t count() const;

private:
    uint64_t * nIncrements; //Index -> nodeId
    uint64_t * nDecrements;
    memdbNodeId_t selfNodeId;
    uint32_t nNodes;
};