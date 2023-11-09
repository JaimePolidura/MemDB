#include "cluster/Cluster.h"

Cluster::Cluster(configuration_t configuration): configuration(configuration) {}

Cluster::Cluster(logger_t logger, configuration_t configuration, onGoingSyncOplogs_t onGoingMultipleResponsesStore, memDbStores_t memDbStores) :
    configuration(configuration),
    memDbStores(memDbStores),
    onGoingMultipleResponsesStore(onGoingMultipleResponsesStore),
    clusterNodes(std::make_shared<ClusterNodes>(configuration, logger)),
    logger(logger)
{}

void Cluster::setBooting() {
    this->selfNode->state = NodeState::BOOTING;
    this->logger->info("Changed cluster node state to BOOTING");
}

auto Cluster::setRunning() -> void {
    this->selfNode->state = NodeState::RUNNING;
    this->logger->info("Changed cluster node state to RUNNING");
}

auto Cluster::fixOplogSegment(uint32_t selfOplogId, uint64_t minTimestamp, uint64_t maxTimestamp) -> std::result<Response> {
    return this->clusterNodes->sendRequestToAnyNode(true, NodePartitionOptions{.partitionId = static_cast<int>(selfOplogId)},[this, selfOplogId, minTimestamp, maxTimestamp](node_t nodeToSend) {
        auto otherNodeOplogId = this->partitions->getOplogIdOfOtherNodeBySelfOplogId(nodeToSend->nodeId, selfOplogId);
        return RequestBuilder::builder()
            .authKey(this->configuration->get(ConfigurationKeys::AUTH_NODE_KEY))
            ->selfNode(this->selfNode->nodeId)
            ->operatorNumber(OperatorNumbers::FIX_OPLOG_SEGMENT)
            ->addArg(SimpleString<memDbDataLength_t>::fromNumber(otherNodeOplogId))
            ->addDoubleArg(minTimestamp)
            ->addDoubleArg(maxTimestamp)
            ->build();
    });
}

auto Cluster::syncOplog(uint64_t lastTimestampProcessedFromOpLog, const NodePartitionOptions options) -> iterator_t<std::result<std::vector<uint8_t>>> {
    return std::make_shared<SyncOplogReceiverIterator>(this->configuration, this->clusterNodes, this->partitions, this->logger, lastTimestampProcessedFromOpLog,
        options.partitionId, [this](){return this->onGoingMultipleResponsesStore->nextSyncOplogId();});
}

auto Cluster::getPartitionObject() -> partitions_t {
    return this->partitions;
}

auto Cluster::broadcastAndWait(const NodePartitionOptions options, const OperationBody& operation) -> multipleResponses_t {
    return this->clusterNodes->broadcastAndWait(options, operation);
}

auto Cluster::broadcast(const NodePartitionOptions options, const OperationBody& operation) -> void {
    this->clusterNodes->broadcast(options, operation);
}

auto Cluster::announceJoin() -> void {
    OperationBody request = RequestBuilder::builder()
        .operatorNumber(OperatorNumbers::JOIN_CLUSTER_ANNOUNCE)
        ->args({SimpleString<memDbDataLength_t>::fromString(
            configuration->get(ConfigurationKeys::ADDRESS) + ":" + configuration->get(ConfigurationKeys::SERVER_PORT))
        })
        ->buildOperationBody();

    this->clusterNodes->broadcastAll(request);
}

auto Cluster::announceLeave() -> void {
    OperationBody request = RequestBuilder::builder()
        .operatorNumber(OperatorNumbers::LEAVE_CLUSTER_ANNOUNCE)
        ->buildOperationBody();

    this->clusterNodes->broadcastAll(request);
}

auto Cluster::getNodeState() -> NodeState {
    return this->selfNode->state;
}

auto Cluster::getNodeId() -> memdbNodeId_t {
    return this->configuration->get<memdbNodeId_t>(ConfigurationKeys::NODE_ID);
}

auto Cluster::getMaxPartitionSize() -> uint32_t {
    if(this->configuration->getBoolean(ConfigurationKeys::USE_PARTITIONS)){
        return this->partitions->getMaxSize();
    } else {
        return 1;
    }
}

auto Cluster::getNodesPerPartition() -> uint32_t {
    if(this->configuration->getBoolean(ConfigurationKeys::USE_PARTITIONS)){
        return this->partitions->getNodesPerPartition();
    } else {
        return 1;
    }
}

auto Cluster::getPartitionIdByKey(SimpleString<memDbDataLength_t> key) -> uint32_t {
    if(this->configuration->getBoolean(ConfigurationKeys::USE_PARTITIONS)){
        return this->partitions->getDistanceOfKey(key);
    } else {
        return 0;
    }
}

auto Cluster::getClusterConfig() -> std::result<GetClusterConfigResponse> {
    std::vector<std::string> addressSeedNodes = this->configuration->getVector(ConfigurationKeys::SEED_NODES);

    for(const std::string& seedNodeAddress : addressSeedNodes){
        Node seedNode = Node{0, seedNodeAddress, NodeState::RUNNING, 5000};
        std::result<Response> responseResult = seedNode.sendRequest(RequestBuilder::builder()
            .authKey(this->configuration->get(ConfigurationKeys::AUTH_NODE_KEY))
            ->operatorNumber(OperatorNumbers::GET_CLUSTER_CONFIG)
            ->selfNode(this->selfNode->nodeId)
            ->build());

        if(responseResult.is_success()){
            uint32_t nodesPerPartition = responseResult->getResponseValueAtOffset(0, 4).to<uint32_t>();
            uint32_t maxPartitionSize = responseResult->getResponseValueAtOffset(4, 4).to<uint32_t>();
            uint32_t nNodesInCluster = responseResult->getResponseValueAtOffset(8, 4).to<uint32_t>();

            int offset = 12;
            std::vector<node_t> nodes = getNodesFromGetClusterConfig(nNodesInCluster, offset, responseResult.get());
            std::vector<RingEntry> ringEntries = getRingEntriesFromGetClusterConfig(nNodesInCluster, offset, responseResult.get());

            return std::ok(GetClusterConfigResponse{
                .nodesPerPartition = nodesPerPartition,
                .maxPartitionSize = maxPartitionSize,
                .nodes = nodes,
                .ringEntries = ringEntries
            });
        }
    }

    return std::error<GetClusterConfigResponse>();
}

std::vector<RingEntry> Cluster::getRingEntriesFromGetClusterConfig(uint32_t nNodesInCluster, int& offset, Response response) {
    std::vector<RingEntry> ringEntries{nNodesInCluster};
    for(int i = 0; i < nNodesInCluster; i++) {
        memdbNodeId_t nodeId = response.getResponseValueAtOffset(offset, 4).to<memdbNodeId_t>();
        uint32_t ringPosition = response.getResponseValueAtOffset(offset + 4, 4).to<uint32_t>();

        ringEntries[i] = RingEntry{.nodeId = nodeId, .ringPosition = ringPosition};

        offset += 8;
    }

    return ringEntries;
}

std::vector<node_t> Cluster::getNodesFromGetClusterConfig(uint32_t nNodesInCluster, int& offset, Response response) {
    std::vector<node_t> nodes{nNodesInCluster};

    for(int i = 0; i < nNodesInCluster; i++) {
        memdbNodeId_t nodeId = response.getResponseValueAtOffset(12, 4).to<memdbNodeId_t>();
        std::size_t sizeAddress = response.getResponseValueAtOffset(16, 4).to<std::size_t>();
        std::string address = response.getResponseValueAtOffset(20, sizeAddress).toString();

        nodes[i] = std::make_shared<Node>(
                nodeId, address, NodeState::RUNNING, this->configuration->get<uint64_t>(ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS)
        );

        offset += 8 + sizeAddress;
    }


    return nodes;
}