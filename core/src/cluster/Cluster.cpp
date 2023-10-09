#include "cluster/Cluster.h"

Cluster::Cluster(logger_t logger, configuration_t configuration) :
    configuration(configuration), clusterDb(std::make_shared<ClusterDb>(configuration, logger)),
    clusterNodes(std::make_shared<ClusterNodes>(configuration, logger)),
    clusterManager(std::make_shared<ClusterManagerService>(configuration, logger)), logger(logger)
{}

void Cluster::setBooting() {
    this->selfNode->state = NodeState::BOOTING;
    this->clusterDb->setNode(this->selfNode->nodeId, this->selfNode);
    this->logger->info("Changed cluster node state to BOOTING");
}

auto Cluster::setRunning() -> void {
    this->selfNode->state = NodeState::RUNNING;
    this->clusterDb->setNode(this->selfNode->nodeId, this->selfNode);
    this->logger->info("Changed cluster node state to RUNNING");
}

auto Cluster::syncOplog(uint64_t lastTimestampProcessedFromOpLog, const NodeGroupOptions options) -> MultiResponseReceiverIterator {
    if(this->clusterNodes->isEmtpy(options)) {
        return MultiResponseReceiverIterator::emtpy();
    }

    int selfOplogIdToSync = options.nodeGroupId;
    OperationLogDeserializer operationLogDeserializer{};
    memdbNodeId_t nodeIdToSendRequest = this->clusterNodes->getRandomNode({}, NodeGroupOptions{.nodeGroupId = selfOplogIdToSync})->nodeId;

    Request initMultiSyncOplogReq = createInitMultiRequestSyncOplog(0x05); //SyncOplog
    Response initMultiSyncOplogRes = clusterNodes->sendRequest(nodeIdToSendRequest, initMultiSyncOplogReq);
    uint64_t nFragments = initMultiSyncOplogRes.responseValue.to<uint64_t>();

    return MultiResponseReceiverIterator(nFragments, [](uint64_t nFragmentId) {
        return std::vector<OperationBody>{};
    });
}

auto Cluster::createInitMultiRequestSyncOplog(uint8_t operatorNumber) -> Request {
    auto authenticationBody = AuthenticationBody{this->configuration->get(ConfigurationKeys::MEMDB_CORE_AUTH_NODE_KEY), false, false};
    auto argsVector = std::make_shared<std::vector<SimpleString<memDbDataLength_t>>>();

    argsVector->push_back(SimpleString<memDbDataLength_t>::fromNumber(operatorNumber));

    OperationBody operationBody{};
    operationBody.args = argsVector;
    operationBody.operatorNumber = 0x06; //InitMulti operator number

    Request request{};
    request.operation = operationBody;
    request.authentication = authenticationBody;
    request.requestNumber = 1; // Used as multi-response Id TODO

    return request;
}

auto Cluster::getUnsyncedOplog(uint64_t lastTimestampProcessedFromOpLog, const NodeGroupOptions options) -> std::vector<OperationBody> {
    if(this->clusterNodes->isEmtpy(options))
        return std::vector<OperationBody>{};

    int selfOplogIdToSync = options.nodeGroupId;
    OperationLogDeserializer operationLogDeserializer{};

    memdbNodeId_t nodeIdToSendRequest = this->clusterNodes->getRandomNode({}, NodeGroupOptions{.nodeGroupId = selfOplogIdToSync})->nodeId;
    Request req = createSyncOplogRequest(lastTimestampProcessedFromOpLog, selfOplogIdToSync, nodeIdToSendRequest);
    Response responseFromSyncData = clusterNodes->sendRequest(nodeIdToSendRequest, req);

    if(!responseFromSyncData.responseValue.hasData())
        return std::vector<OperationBody>{};

    uint8_t * begin = responseFromSyncData.responseValue.data();
    auto bytes = std::vector<uint8_t>(begin, begin + responseFromSyncData.responseValue.size);

    return operationLogDeserializer.deserializeAll(bytes);
}

auto Cluster::getPartitionObject() -> partitions_t {
    return this->partitions;
}

auto Cluster::broadcast(const OperationBody& operation) -> void {
    this->clusterNodes->broadcast(operation);
}

auto Cluster::getNodeState() -> NodeState {
    return this->selfNode->state;
}

auto Cluster::getNodeId() -> memdbNodeId_t {
    return this->selfNode->nodeId;
}

auto Cluster::watchForChangesInNodesClusterDb(std::function<void(node_t nodeChanged, ClusterDbChangeType changeType)> onChangeCallback) -> void {
    this->clusterDb->watchNodeChanges([this, onChangeCallback](ClusterDbValueChanged nodeChangedEvent) {
        auto node = Node::fromJson(nodeChangedEvent.value);
        auto selfNodeChanged = node->nodeId == this->configuration->get<memdbNodeId_t>(ConfigurationKeys::MEMDB_CORE_NODE_ID);

        if (!selfNodeChanged) {
            onChangeCallback(node, nodeChangedEvent.changeType);
        }
        if (selfNodeChanged && node->state == NodeState::SHUTDOWN) { //Reload
            this->setRunning();
        }
    });
}

auto Cluster::createSyncOplogRequest(uint64_t timestamp, uint32_t selfOplogId, memdbNodeId_t nodeIdToSendRequest) -> Request {
    auto authenticationBody = AuthenticationBody{this->configuration->get(ConfigurationKeys::MEMDB_CORE_AUTH_NODE_KEY), false, false};
    auto argsVector = std::make_shared<std::vector<SimpleString<memDbDataLength_t>>>();
    auto nodeOplogId = this->configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_PARTITIONS) ?
                       (this->partitions->isClockwiseNeighbor(nodeIdToSendRequest) ?
                        selfOplogId + this->partitions->getDistanceClockwise(nodeIdToSendRequest) :
                        selfOplogId - this->partitions->getDistanceCounterClockwise(nodeIdToSendRequest)) : 0;

    uint32_t part1 = timestamp >> 32;
    uint32_t part2 = (uint32_t) timestamp & 0xFFFFFFFF;

    argsVector->push_back(SimpleString<memDbDataLength_t>::fromNumber(part1));
    argsVector->push_back(SimpleString<memDbDataLength_t>::fromNumber(part2));
    argsVector->push_back(SimpleString<memDbDataLength_t>::fromNumber(nodeOplogId));

    OperationBody operationBody{};
    operationBody.args = argsVector;
    operationBody.operatorNumber = 0x05; //SyncOplogOperator operator number

    Request request{};
    request.operation = operationBody;
    request.authentication = authenticationBody;

    return request;
}