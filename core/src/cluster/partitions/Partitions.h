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
    Partitions(const std::vector<RingEntry>& allRingEntries, uint32_t nodesPerPartition, uint32_t maxSize, configuration_t configuration):
        nodesPerPartition(nodesPerPartition), maxSize(maxSize), ringEntries(RingEntries::fromEntries(allRingEntries)), configuration(configuration) {
        this->selfEntry = this->ringEntries.getByNodeId(configuration->get<memdbNodeId_t>(ConfigurationKeys::MEMDB_CORE_NODE_ID));
    }

    Partitions() = default;

    uint32_t getRingPositionByKey(SimpleString<memDbDataLength_t> key) const {
        return HashCalculator::calculate(key.toString()) % this->maxSize;
    }

    // self --> (clockwise) nodeB
    bool isClockwiseNeighbor(memdbNodeId_t nodeB) {
        return this->ringEntries.getDistanceClockwise(this->selfEntry.nodeId, nodeB) < this->nodesPerPartition;
    }

    std::vector<RingEntry> getNeighborsClockwise(int numberNeighbors = -1) {
        return this->ringEntries.getNeighborsClockwise(this->selfEntry.nodeId,numberNeighbors == -1 ? this->nodesPerPartition - 1 : numberNeighbors);
    }

    std::vector<RingEntry> getNeighborsClockwiseByNodeId(memdbNodeId_t nodeId) {
        return this->ringEntries.getNeighborsClockwise(nodeId, this->nodesPerPartition - 1);
    }

    RingEntry getNeighborCounterClockwiseByNodeId(memdbNodeId_t nodeId) {
        return this->ringEntries.getNeighborCounterClockwise(nodeId);
    }

    uint32_t getDistanceOfKey(SimpleString<memDbDataLength_t> key) {
        uint32_t ringPosition = this->getRingPositionByKey(key);
        uint32_t nodeThatWouldHoldThatKey = this->ringEntries.getRingEntryBelongsToPosition(ringPosition).nodeId;

        return std::abs(this->ringEntries.getDistance(this->selfEntry.nodeId, nodeThatWouldHoldThatKey));
    }

    bool canHoldKey(SimpleString<memDbDataLength_t> key) {
        return this->getDistanceOfKey(key) < this->nodesPerPartition;
    }

    virtual int getDistance(memdbNodeId_t nodeB) {
        return this->ringEntries.getDistance(this->selfEntry.nodeId, nodeB);
    }

    // self <-- (counter clockwise) nodeB
    uint32_t getDistanceCounterClockwise(memdbNodeId_t nodeB) {
        return this->ringEntries.getDistanceCounterClockwise(this->selfEntry.nodeId, nodeB);
    }

    // self --> (clockwise) nodeB
    uint32_t getDistanceClockwise(memdbNodeId_t nodeB) {
        return this->ringEntries.getDistanceClockwise(this->selfEntry.nodeId, nodeB);
    }

    bool isNeighbor(memdbNodeId_t otherNode) {
        return std::abs(this->ringEntries.getDistance(this->selfEntry.nodeId, otherNode)) < this->nodesPerPartition;
    }

    void deleteByNodeId(memdbNodeId_t nodeId) {
        this->ringEntries.deleteByNodeId(nodeId);
    }

    void add(RingEntry ringEntry){
        this->ringEntries.add(ringEntry);
    }

    uint32_t getNodesPerPartition() const {
        return this->nodesPerPartition;
    }

    RingEntry getByNodeId(memdbNodeId_t nodeId) {
        return this->ringEntries.getByNodeId(nodeId);
    }

    RingEntry getSelfEntry() {
        return this->selfEntry;
    }
};

using partitions_t = std::shared_ptr<Partitions>;