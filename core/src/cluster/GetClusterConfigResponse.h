#pragma once

#include "shared.h"
#include "cluster/Node.h"
#include "cluster/partitions/RingEntry.h"

struct GetClusterConfigResponse {
    uint32_t nodesPerPartition;
    uint32_t maxPartitionSize;
    std::vector<node_t> nodes;
    std::vector<RingEntry> ringEntries;
};