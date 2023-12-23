#pragma once

#include "memdbtypes.h"
#include "shared.h"
#include "utils/threads/SharedLock.h"
#include "db/counters/ReplicateCounterRequest.h"
#include "ReplicateCounterResponse.h"
#include "db/counters/CounterArrayIndexMapper.h"

class Counter {
public:
    Counter(memdbNodeId_t selfNodeId, uint32_t nNodes);

    uint64_t increment();

    uint64_t decrement();

    uint64_t count() const;

    void syncIncrement(uint64_t valueToSync, memdbNodeId_t otherNodeId);

    void syncDecrement(uint64_t value, memdbNodeId_t otherNodeId);

    std::pair<uint64_t, uint64_t> getLastSeen(memdbNodeId_t nodeId) const;

    ReplicateCounterResponse onReplicationCounter(ReplicateCounterRequest request);

private:
    std::shared_ptr<SharedLock> lock;

    std::shared_ptr<CounterArrayIndexMapper> arrayNodeIndexMapper{};

    uint64_t * nIncrements; //Index -> nodeId
    uint64_t * nDecrements;
    memdbNodeId_t selfNodeId;
    uint32_t nNodes;

    void maybeResizeCounters(memdbNodeId_t otherNodeId);

    void updateCounterFromReplication(ReplicateCounterRequest request);

    int getIndexByNode(memdbNodeId_t nodeId) const;
};