#pragma once

#include "cluster/partitions/Partitions.h"

class PartitionTest {
private:
    partitions_t partitions;

public:
    explicit PartitionTest(partitions_t partitions): partitions(partitions) {}

    void setNodesPerPartition(uint32_t nodesPerPartition) {
        partitions->nodesPerPartition = nodesPerPartition;
    }

    void setSelfRingEntry(RingEntry selfEntry) {
        this->partitions->selfEntry = selfEntry;
    }

    void setRingEntries(std::vector<RingEntry> ringEntries) {
        partitions->ringEntries = RingEntries::fromEntries(ringEntries);
    }
};