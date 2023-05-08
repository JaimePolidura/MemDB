#pragma once

#include "shared.h"

#include "utils/crypto/HashCalculator.h"

#include "cluster/partitions/RingEntry.h"
#include "cluster/partitions/RingEntries.h"

#include "config/Configuration.h"

class Partitions {
private:
    RingEntries ringEntries;

    RingEntry selfEntry;
    uint32_t nodesPerPartition;
    uint32_t maxSize;

    configuration_t configuration;

public:
    Partitions(const std::vector<RingEntry>& allRingEntries, uint32_t nodesPerPartition, uint32_t maxSize, configuration_t configuration):
        nodesPerPartition(nodesPerPartition), maxSize(maxSize), ringEntries(RingEntries::fromEntries(allRingEntries)), configuration(configuration) {
        this->selfEntry = this->ringEntries.getByNodeId(configuration->get<memdbNodeId_t>(ConfigurationKeys::MEMDB_CORE_NODE_ID));
    }

    Partitions() = default;

    uint32_t getRingPositionByKey(SimpleString<memDbDataLength_t> key) {
        return HashCalculator::calculate(key.toString()) % this->maxSize;
    }

    // self --> (clockwise) nodeB
    bool isClockwiseNeighbor(memdbNodeId_t nodeB) {
        return this->ringEntries.getDistanceClockwise(this->selfEntry.nodeId, nodeB) <= this->nodesPerPartition;
    }

    std::vector<RingEntry> getNeighborsClockwise(int numberNeighbors = this->nodesPerPartition) {
        return this->ringEntries.getNeighborsClockwise(this->selfEntry.nodeId, this->nodesPerPartition);
    }

    // self --> (clockwise) nodeB
    bool isCounterClockwiseNeighbor(memdbNodeId_t nodeB) {
        return this->ringEntries.getDistanceCounterClockwise(this->selfEntry.nodeId, nodeB) <= this->nodesPerPartition;
    }

    uint32_t getDistanceOfKey(SimpleString<memDbDataLength_t> key) {
        uint32_t ringPosition = this->getRingPositionByKey(key);
        uint32_t nodeThatWouldHoldThatKey = this->ringEntries.getRingEntryBelongsToPosition(ringPosition).nodeId;

        return this->ringEntries.getDistance(this->selfEntry.nodeId, nodeThatWouldHoldThatKey);
    }

    bool canHoldKey(SimpleString<memDbDataLength_t> key) {
        return this->getDistanceOfKey(key) <= this->nodesPerPartition;
    }

    // self <-- (counter clockwise) nodeB
    uint32_t getDistanceCounterClockwise(memdbNodeId_t nodeB) {
        return this->ringEntries.getDistanceCounterClockwise(this->selfEntry.nodeId, nodeB);
    }

    // self --> (clockwise) nodeB
    uint32_t getDistanceClockwise(memdbNodeId_t nodeB) {
        return this->ringEntries.getDistanceClockwise(this->selfEntry.nodeId, nodeB);
    }

    uint32_t getDistance(memdbNodeId_t otherNode) {
        return this->ringEntries.getDistance(this->selfEntry.nodeId, otherNode);
    }

    bool isNeighbor(memdbNodeId_t otherNode) {
        return this->ringEntries.getDistance(this->selfEntry.nodeId, otherNode) <= this->nodesPerPartition;
    }

    uint32_t getNodesPerPartition() {
        return this->nodesPerPartition;
    }

    void add(RingEntry ringEntry){
        this->ringEntries.add(ringEntry);
    }
};

using partitions_t = std::shared_ptr<Partitions>;