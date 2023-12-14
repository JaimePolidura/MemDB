#pragma once

#include "shared.h"
#include "cluster/Node.h"
#include "cluster/partitions/RingEntry.h"

struct GetClusterConfigResponse {
public:
    uint32_t nodesPerPartition;
    uint32_t maxPartitionSize;
    std::vector<node_t> nodes;
    std::vector<RingEntry> ringEntries;

    std::vector<uint8_t> serialize() const;

    static GetClusterConfigResponse deserialize(const std::vector<uint8_t>& bytes, configuration_t configuration);

private:
    std::size_t getSerializedBytesSize() const;

    static std::vector<RingEntry> getRingEntriesFromGetClusterConfig(uint32_t nNodesInCluster, int& offset, const std::vector<uint8_t>& bytes);
    static std::vector<node_t> getNodesFromGetClusterConfig(int nNodesInCluster, int& offset, const std::vector<uint8_t>& bytes, configuration_t configuration);
};