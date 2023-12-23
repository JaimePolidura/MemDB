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

    int64_t increment();

    int64_t decrement();

    int64_t count() const;

    void syncIncrement(int64_t valueToSync, memdbNodeId_t otherNodeId);

    void syncDecrement(int64_t value, memdbNodeId_t otherNodeId);

    std::pair<int64_t, int64_t> getLastSeen(memdbNodeId_t nodeId) const;

    ReplicateCounterResponse onReplicationCounter(ReplicateCounterRequest request);

private:
    std::shared_ptr<SharedLock> lock;

    std::shared_ptr<CounterArrayIndexMapper> arrayNodeIndexMapper{};

    int64_t * nIncrements; //Index -> nodeId
    int64_t * nDecrements;
    memdbNodeId_t selfNodeId;
    uint32_t nNodes;

    void maybeResizeCounters(memdbNodeId_t otherNodeId);

    void updateCounterFromReplication(ReplicateCounterRequest request);

    int getIndexByNode(memdbNodeId_t nodeId) const;
};