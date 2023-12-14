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
    this->selfState = NodeState::BOOTING;
    this->logger->info("Changed cluster node state to BOOTING");
}

auto Cluster::setRunning() -> void {
    this->selfState = NodeState::RUNNING;
    this->logger->info("Changed cluster node state to RUNNING");
}

auto Cluster::fixOplogSegment(uint32_t selfOplogId, uint64_t minTimestamp, uint64_t maxTimestamp) -> std::result<Response> {
    return this->clusterNodes->sendRequestToAnyNode(true, SendRequestOptions{.partitionId = static_cast<int>(selfOplogId)},[this, selfOplogId, minTimestamp, maxTimestamp](node_t nodeToSend) {
        auto otherNodeOplogId = this->partitions->getOplogIdOfOtherNodeBySelfOplogId(nodeToSend->nodeId, selfOplogId);
        return RequestBuilder::builder()
            .authKey(this->configuration->get(ConfigurationKeys::AUTH_NODE_KEY))
            ->selfNode(this->getNodeId())
            ->operatorNumber(OperatorNumbers::FIX_OPLOG_SEGMENT)
            ->addArg(SimpleString<memDbDataLength_t>::fromNumber(otherNodeOplogId))
            ->addDoubleArg(minTimestamp)
            ->addDoubleArg(maxTimestamp)
            ->build();
    });
}

auto Cluster::syncOplog(uint64_t lastTimestampProcessedFromOpLog, const SendRequestOptions options) -> iterator_t<std::result<std::vector<uint8_t>>> {
    return std::make_shared<SyncOplogReceiverIterator>(this->configuration, this->clusterNodes, this->partitions, this->logger, lastTimestampProcessedFromOpLog,
        options.partitionId, [this](){return this->onGoingMultipleResponsesStore->nextSyncOplogId();});
}

auto Cluster::getPartitionObject() -> partitions_t {
    return this->partitions;
}

auto Cluster::broadcastAndWait(const OperationBody& operation, SendRequestOptions options) -> multipleResponses_t {
    return this->clusterNodes->broadcastAndWait(operation, options);
}

auto Cluster::checkHintedHandoff(memdbNodeId_t nodeId) -> void {
    this->clusterNodes->sendHintedHandoff(nodeId);
}

auto Cluster::broadcast(const OperationBody& operation, SendRequestOptions options) -> void {
    this->clusterNodes->broadcast(operation, options);
}

auto Cluster::announceJoin() -> void {
    OperationBody request = RequestBuilder::builder()
        .operatorNumber(OperatorNumbers::JOIN_CLUSTER_ANNOUNCE)
        ->args({SimpleString<memDbDataLength_t>::fromString(
            configuration->get(ConfigurationKeys::ADDRESS) + ":" + configuration->get(ConfigurationKeys::SERVER_PORT))
        })
        ->buildOperationBody();

    this->logger->info("Announcing join to the cluster");

    this->clusterNodes->broadcastAll(request, SendRequestOptions{.canBeStoredInHint = true});
}

auto Cluster::announceLeave() -> void {
    OperationBody request = RequestBuilder::builder()
        .operatorNumber(OperatorNumbers::LEAVE_CLUSTER_ANNOUNCE)
        ->buildOperationBody();

    this->logger->info("Announcing leave to the cluster");

    this->clusterNodes->broadcastAll(request, SendRequestOptions{.canBeStoredInHint = true});
}

auto Cluster::getNodeState() -> NodeState {
    return this->selfState;
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
    std::vector<node_t> candidatesNodes = this->getCandidatesToSendGetClusterConfig();
    uint64_t timeoutMs = this->configuration->get<uint64_t>(ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS);
    uint32_t nRetries = this->configuration->get<uint32_t>(ConfigurationKeys::NODE_REQUEST_N_RETRIES);

    if(candidatesNodes.empty()) {
        return std::error<GetClusterConfigResponse>();
    }

    this->logger->info("Sending GET_CLUSTER_CONFIG to any seed node");

    for(const node_t candidateNode : candidatesNodes) {
        if(candidateNode->address == this->configuration->get(ConfigurationKeys::ADDRESS) + ":" +
            this->configuration->get(ConfigurationKeys::SERVER_PORT))  {
            continue;
        }

        for(int i = 0; i < nRetries; i++) {
            std::result<Response> responseResult = candidateNode->sendRequest(RequestBuilder::builder()
                .authKey(this->configuration->get(ConfigurationKeys::AUTH_NODE_KEY))
                ->operatorNumber(OperatorNumbers::GET_CLUSTER_CONFIG)
                ->selfNode(this->getNodeId())
                ->build());

            candidateNode->closeConnection();

            if(responseResult.is_success() && responseResult->isSuccessful){
                this->logger->info("Received GET_CLUSTER_CONFIG from seed node {0}", candidateNode->nodeId);

                GetClusterConfigResponse response = GetClusterConfigResponse::deserialize(responseResult.get().responseValue.toVector(), configuration);
                this->persistClusterConfig(response);

                return std::ok(response);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs));
        }
    }

    return std::error<GetClusterConfigResponse>();
}

std::vector<node_t> Cluster::getCandidatesToSendGetClusterConfig() {
    std::string clusterConfigPath = configuration->get(ConfigurationKeys::DATA_PATH)
        + "/cluster-config";
    if(!FileUtils::exists(clusterConfigPath)) {
        FileUtils::createFile(configuration->get(ConfigurationKeys::DATA_PATH), "cluster-config");
        return getSeedNodes();
    }

    std::vector<uint8_t> bytesClusterConfigRespnose = FileUtils::readBytes(clusterConfigPath);

    try {
        return GetClusterConfigResponse::deserialize(bytesClusterConfigRespnose, configuration).nodes;
    }catch (const std::exception& e) {
        this->logger->error("Error while reading cluster config file {0} it might be corrupted", clusterConfigPath);
        return getSeedNodes();
    }
}

void Cluster::persistClusterConfig(const GetClusterConfigResponse& response) {
    std::vector<uint8_t> serialized = response.serialize();
    std::string clusterConfigPath = configuration->get(ConfigurationKeys::DATA_PATH)
        + "/cluster-config";

    FileUtils::clear(clusterConfigPath);
    FileUtils::writeBytes(clusterConfigPath, serialized);
}

std::vector<node_t> Cluster::getSeedNodes() {
    std::vector<std::string> seedNodesAddress = this->configuration->getVector(ConfigurationKeys::SEED_NODES);
    std::vector<node_t> nodes{seedNodesAddress.size()};
    uint64_t timeout = configuration->get<uint64_t>(ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS);

    for(int i = 0; i < nodes.size(); i++) {
        nodes[i] = std::make_shared<Node>(0, seedNodesAddress[i], timeout);
    }

    return nodes;
}