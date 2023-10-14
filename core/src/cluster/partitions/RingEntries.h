#pragma once

#include "cluster/partitions/RingEntry.h"
#include "cluster/Node.h"

struct RingEntryNode {
    RingEntry entry;

    RingEntryNode * next;
    RingEntryNode * back;

    RingEntryNode() = default;

    RingEntryNode(RingEntry entry): entry(entry) {}

    memdbNodeId_t getNodeId() {
        return this->entry.nodeId;
    }
};

class RingEntries {
private:
    RingEntryNode * head;
    std::map<memdbNodeId_t, RingEntryNode *> indexByNodeId;

public:
    RingEntries(RingEntryNode * head, const std::map<memdbNodeId_t, RingEntryNode *>& indexByNodeId): head(head), indexByNodeId(std::move(indexByNodeId)) {}

    RingEntries() = default;

    void add(RingEntry ringEntryToAdd);

    void deleteByNodeId(memdbNodeId_t nodeId);

    std::vector<RingEntry> getNeighborsClockwise(memdbNodeId_t nodeId, uint32_t numberNeighbors);

    std::vector<RingEntry> getNeighborsCounterClockwise(memdbNodeId_t nodeId, uint32_t numberNeighbors);

    RingEntry getNeighborCounterClockwise(memdbNodeId_t nodeId);

    RingEntry getRingEntryBelongsToPosition(uint32_t ringPosition);

    // nodeA --> nodeB -> positive
    // nodeB --> nodeA -> negative
    int getDistance(memdbNodeId_t nodeA, memdbNodeId_t nodeB);

    // nodeA <-- (memdb_thread_pool_counter clockwise) nodeB
    uint32_t getDistanceCounterClockwise(memdbNodeId_t nodeA, memdbNodeId_t nodeB);

    // nodeA --> (clockwise) nodeB
    uint32_t getDistanceClockwise(memdbNodeId_t nodeA, memdbNodeId_t nodeB);

    RingEntry getByNodeId(memdbNodeId_t nodeId);

    static RingEntries fromEntries(std::vector<RingEntry> entries);
};