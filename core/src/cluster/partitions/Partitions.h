#pragma once

#include "shared.h"

#include "utils/crypto/HashCalculator.h"

#include "cluster/partitions/RingEntry.h"
#include "cluster/partitions/RingEntries.h"

#include "config/Configuration.h"

class Partitions {
private:
    RingEntries ringEntries;

    RingEntry selfEntry{};
    uint32_t nodesPerPartition{};
    uint32_t maxSize{};

    configuration_t configuration;

    friend class PartitionTest;

public:
    Partitions(const std::vector<RingEntry>& allRingEntries, uint32_t nodesPerPartition, uint32_t maxSize, configuration_t configuration);

    Partitions() = default;

    uint32_t getRingPositionByKey(SimpleString<memDbDataLength_t> key) const;

    uint32_t getRingPositionByNodeId(memdbNodeId_t nodeId);

    // self --> (clockwise) nodeB
    bool isClockwiseNeighbor(memdbNodeId_t nodeB);

    std::vector<RingEntry> getNeighborsClockwise(int numberNeighbors = -1);

    std::vector<RingEntry> getNeighborsClockwiseByNodeId(memdbNodeId_t nodeId);

    std::vector<RingEntry> getNeighborsCounterClockwiseByNodeId(memdbNodeId_t nodeId);

    std::vector<RingEntry> getNeighbors();

    RingEntry getNeighborCounterClockwiseByNodeId(memdbNodeId_t nodeId);

    RingEntry getNeighborClockwiseByNodeId(memdbNodeId_t nodeId);

    uint32_t getDistanceOfKey(SimpleString<memDbDataLength_t> key);

    bool canHoldKey(SimpleString<memDbDataLength_t> key);

    virtual int getDistance(memdbNodeId_t nodeB);

    // self <-- (memdb_thread_pool_counter clockwise) nodeB
    uint32_t getDistanceCounterClockwise(memdbNodeId_t nodeB);

    // self --> (clockwise) nodeB
    uint32_t getDistanceClockwise(memdbNodeId_t nodeB);

    bool isNeighbor(memdbNodeId_t otherNode);

    void deleteByNodeId(memdbNodeId_t nodeId);

    void add(RingEntry ringEntry);

    virtual uint32_t getNodesPerPartition() const;

    RingEntry getByNodeId(memdbNodeId_t nodeId);

    RingEntry getSelfEntry();
};

using partitions_t = std::shared_ptr<Partitions>;