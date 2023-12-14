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
        Utils::appendToBuffer(node->address, serialized);
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

GetClusterConfigResponse GetClusterConfigResponse::deserialize(const Response& response, configuration_t configuration) {
    uint32_t nodesPerPartition = response.getResponseValueAtOffset(0, 4).to<uint32_t>();
    uint32_t maxPartitionSize = response.getResponseValueAtOffset(4, 4).to<uint32_t>();
    int nNodesInCluster = response.getResponseValueAtOffset(8, 4).to<int>();
    bool usingPartitions = configuration->getBoolean(ConfigurationKeys::USE_PARTITIONS);

    int offset = 12;
    std::vector<node_t> nodes = getNodesFromGetClusterConfig(nNodesInCluster, offset, response, configuration);
    std::vector<RingEntry> ringEntries = usingPartitions ? //Using partitions
        getRingEntriesFromGetClusterConfig(nNodesInCluster, offset, response) :
        std::vector<RingEntry>{};

    return GetClusterConfigResponse{
        .nodesPerPartition = nodesPerPartition,
        .maxPartitionSize = maxPartitionSize,
        .nodes = nodes,
        .ringEntries = ringEntries,
    };
}

std::vector<RingEntry> GetClusterConfigResponse::getRingEntriesFromGetClusterConfig(uint32_t nNodesInCluster, int& offset, Response response) {
    std::vector<RingEntry> ringEntries{nNodesInCluster};
    for(int i = 0; i < nNodesInCluster; i++) {
        memdbNodeId_t nodeId = response.getResponseValueAtOffset(offset, sizeof(memdbNodeId_t)).to<memdbNodeId_t>();
        uint32_t ringPosition = response.getResponseValueAtOffset(offset + sizeof(memdbNodeId_t), 4).to<uint32_t>();

        ringEntries[i] = RingEntry{.nodeId = nodeId, .ringPosition = ringPosition};

        offset += sizeof(memdbNodeId_t) + 4;
    }

    return ringEntries;
}

std::vector<node_t> GetClusterConfigResponse::getNodesFromGetClusterConfig(int nNodesInCluster, int& offset, Response response, configuration_t configuration) {
    uint64_t timeout = configuration->get<uint64_t>(ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS);
    std::vector<node_t> nodes{};

    for(int i = 0; i < nNodesInCluster; i++) {
        memdbNodeId_t nodeId = response.getResponseValueAtOffset(offset, sizeof(memdbNodeId_t)).to<memdbNodeId_t>();
        uint32_t sizeAddress = response.getResponseValueAtOffset(offset + sizeof(memdbNodeId_t), 4).to<uint32_t>();
        std::string address = response.getResponseValueAtOffset(offset + sizeof(memdbNodeId_t) + 4, sizeAddress).toString();

        nodes.push_back(std::make_shared<Node>(
                nodeId, address, timeout
        ));

        offset += sizeof(nodeId) + 4 + sizeAddress;
    }

    return nodes;
}