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

    uint32_t getDistanceOfKey(SimpleString<memDbDataLength_t> key) {
        uint32_t ringPosition = this->getRingPosition(key);
        uint32_t nodeThatWouldHoldThatKey = this->ringEntries.getRingEntryBelongsToPosition(ringPosition).nodeId;

        return this->ringEntries.getDistance(this->selfEntry.nodeId, nodeThatWouldHoldThatKey);
    }

    bool canHoldKey(SimpleString<memDbDataLength_t> key) {
        return this->getDistanceOfKey(key) <= this->nodesPerPartition;
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

private:
    uint32_t getRingPosition(SimpleString<memDbDataLength_t> key) {
        return HashCalculator::calculate(key.toString()) % this->maxSize;
    }
};

using partitions_t = std::shared_ptr<Partitions>;