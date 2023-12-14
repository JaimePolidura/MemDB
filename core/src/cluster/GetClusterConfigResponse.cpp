#include "GetClusterConfigResponse.h"

std::vector<uint8_t> GetClusterConfigResponse::serialize() const {
    std::size_t serializeSize = this->getSerializedBytesSize();
    std::vector<uint8_t> serialized{};
    serialized.reserve(serializeSize);

    Utils::appendToBuffer(nodesPerPartition, serialized);
    Utils::appendToBuffer(maxPartitionSize, serialized);
    Utils::appendToBuffer(static_cast<uint32_t>(nodes.size()), serialized);
    for (node_t node : nodes) {
        Utils::appendToBuffer(node->nodeId, serialized);
        Utils::appendToBuffer(static_cast<uint32_t>(node->address.size()), serialized);
        Utils::appendToBuffer(reinterpret_cast<uint8_t *>(node->address.data()), node->address.size(), serialized);
    }
    for (RingEntry ringEntry : ringEntries) {
        Utils::appendToBuffer(ringEntry.nodeId, serialized);
        Utils::appendToBuffer(ringEntry.ringPosition, serialized);
    }

    return serialized;
}

std::size_t GetClusterConfigResponse::getSerializedBytesSize() const {
    std::size_t size = 4 + 4; //nodesPerPartition, maxPartitionSize
    size += 4; //Nodes size

    for(int i = 0; i < nodes.size(); i++) { //Nodes
        size += sizeof(memdbNodeId_t) + 4 + nodes[i]->address.size();
    }
    for(int i = 0; i < ringEntries.size(); i++) { //Nodes
        size += sizeof(memdbNodeId_t) + sizeof(uint32_t);
    }

    return size;
}

GetClusterConfigResponse GetClusterConfigResponse::deserialize(const std::vector<uint8_t>& bytes, configuration_t configuration) {
    uint32_t nodesPerPartition = Utils::parseFromBuffer<uint32_t>(bytes, 0);
    uint32_t maxPartitionSize = Utils::parseFromBuffer<uint32_t>(bytes, 4);
    int nNodesInCluster = Utils::parseFromBuffer<uint32_t>(bytes, 8);
    bool usingPartitions = configuration->getBoolean(ConfigurationKeys::USE_PARTITIONS);

    int offset = 12;
    std::vector<node_t> nodes = getNodesFromGetClusterConfig(nNodesInCluster, offset, bytes, configuration);
    std::vector<RingEntry> ringEntries = usingPartitions ? //Using partitions
        getRingEntriesFromGetClusterConfig(nNodesInCluster, offset, bytes) :
        std::vector<RingEntry>{};

    return GetClusterConfigResponse{
        .nodesPerPartition = nodesPerPartition,
        .maxPartitionSize = maxPartitionSize,
        .nodes = nodes,
        .ringEntries = ringEntries,
    };
}

std::vector<RingEntry> GetClusterConfigResponse::getRingEntriesFromGetClusterConfig(uint32_t nNodesInCluster, int& offset, const std::vector<uint8_t>& bytes) {
    std::vector<RingEntry> ringEntries{nNodesInCluster};
    for(int i = 0; i < nNodesInCluster; i++) {
        memdbNodeId_t nodeId = Utils::parseFromBuffer<memdbNodeId_t>(bytes, offset);
        uint32_t ringPosition = Utils::parseFromBuffer<uint32_t>(bytes, offset + sizeof(memdbNodeId_t));

        ringEntries[i] = RingEntry{.nodeId = nodeId, .ringPosition = ringPosition};

        offset += sizeof(memdbNodeId_t) + 4;
    }

    return ringEntries;
}

std::vector<node_t> GetClusterConfigResponse::getNodesFromGetClusterConfig(int nNodesInCluster, int& offset, const std::vector<uint8_t>& bytes, configuration_t configuration) {
    uint64_t timeout = configuration->get<uint64_t>(ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS);
    std::vector<node_t> nodes{};

    for(int i = 0; i < nNodesInCluster; i++) {
        memdbNodeId_t nodeId = Utils::parseFromBuffer<memdbNodeId_t>(bytes, offset);
        uint32_t sizeAddress = Utils::parseFromBuffer<uint32_t>(bytes, offset + sizeof(memdbNodeId_t));
        std::string address = std::string((char *)(bytes.data() + offset + sizeof(memdbNodeId_t) + 4), sizeAddress);

        nodes.push_back(std::make_shared<Node>(
                nodeId, address, timeout
        ));

        offset += sizeof(nodeId) + 4 + sizeAddress;
    }

    return nodes;
}