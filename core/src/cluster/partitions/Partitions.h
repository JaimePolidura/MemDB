#pragma once

#include "shared.h"
#include "cluster/partitions/RingEntry.h"

class Partitions {
private:
    std::vector<RingEntry> allRingEntries;
    uint32_t nodesPerPartition;
    uint32_t maxSize;

public:
    Partitions() = default;

    Partitions(const std::vector<RingEntry>& allRingEntries, uint32_t nodesPerPartition, uint32_t maxSize):
        allRingEntries(allRingEntries), nodesPerPartition(nodesPerPartition), maxSize(maxSize) {}

};

using partitions_t = std::shared_ptr<Partitions>;