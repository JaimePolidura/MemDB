#include "cluster/Cluster.h"

Cluster::Cluster(logger_t logger, configuration_t configuration, onGoingMultipleResponsesStore_t onGoingMultipleResponsesStore) :
    configuration(configuration),
    clusterDb(std::make_shared<ClusterDb>(configuration, logger)),
    onGoingMultipleResponsesStore(onGoingMultipleResponsesStore),
    clusterNodes(std::make_shared<ClusterNodes>(configuration, logger)),
    clusterManager(std::make_shared<ClusterManagerService>(configuration, logger)),
    logger(logger)
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

auto Cluster::syncOplog(uint64_t lastTimestampProcessedFromOpLog, const NodeGroupOptions options) -> multiResponseReceiverIterator_t {
    if(this->clusterNodes->isEmtpy(options)) {
        return MultiResponseReceiverIterator::emtpy();
    }

    int selfOplogIdToSync = options.nodeGroupId;
    memdbNodeId_t nodeIdToSendRequest = this->clusterNodes->getRandomNode({}, NodeGroupOptions{.nodeGroupId = selfOplogIdToSync})->nodeId;

    Request initMultiSyncOplogReq = createSyncOplogRequestInitMultiResponse(lastTimestampProcessedFromOpLog, selfOplogIdToSync, nodeIdToSendRequest); //SyncOplog
    Response initMultiSyncOplogRes = clusterNodes->sendRequest(nodeIdToSendRequest, initMultiSyncOplogReq);
    uint64_t nFragments = initMultiSyncOplogRes.responseValue.to<uint64_t>();
    uint64_t multiResponseId = initMultiSyncOplogReq.requestNumber;

    return std::make_shared<MultiResponseReceiverIterator>(multiResponseId, nFragments, [this, nodeIdToSendRequest](uint64_t multiResponseId, uint64_t nFragmentId) {
        Request request = this->createNextFragmentMultiResponseRequest(multiResponseId);
        Response response = this->clusterNodes->sendRequest(nodeIdToSendRequest, request);

        return response.responseValue.toVector();
    });
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

auto Cluster::createSyncOplogRequestInitMultiResponse(uint64_t timestamp, uint32_t selfOplogId, memdbNodeId_t nodeIdToSendRequest) -> Request {
    auto authenticationBody = AuthenticationBody{this->configuration->get(ConfigurationKeys::MEMDB_CORE_AUTH_NODE_KEY), false, false};
    auto argsVector = std::make_shared<std::vector<SimpleString<memDbDataLength_t>>>();
    uint32_t part1 = timestamp >> 32;
    uint32_t part2 = (uint32_t) timestamp & 0xFFFFFFFF;
    uint32_t nodeOplogId = this->configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_PARTITIONS) ?
                           (this->partitions->isClockwiseNeighbor(nodeIdToSendRequest) ?
                            selfOplogId + this->partitions->getDistanceClockwise(nodeIdToSendRequest) :
                            selfOplogId - this->partitions->getDistanceCounterClockwise(nodeIdToSendRequest)) : 0;

    argsVector->push_back(SimpleString<memDbDataLength_t>::fromNumber(0x05)); //Sync oplog operator ID
    argsVector->push_back(SimpleString<memDbDataLength_t>::fromNumber(part1));
    argsVector->push_back(SimpleString<memDbDataLength_t>::fromNumber(part2));
    argsVector->push_back(SimpleString<memDbDataLength_t>::fromNumber(nodeOplogId));

    OperationBody operationBody{};
    operationBody.args = argsVector;
    operationBody.operatorNumber = 0x06; //InitMulti operator number

    Request request{};
    request.operation = operationBody;
    request.authentication = authenticationBody;
    request.requestNumber = this->onGoingMultipleResponsesStore->nextMultiResponseId(); // Used as multi-response Id

    return request;
}

auto Cluster::createNextFragmentMultiResponseRequest(uint64_t multiResponseId) -> Request {
    OperationBody operationBody{};
    operationBody.operatorNumber = 0x07; //Next fragment multi-response

    Request request{};
    request.operation = operationBody;
    request.authentication = AuthenticationBody{this->configuration->get(ConfigurationKeys::MEMDB_CORE_AUTH_NODE_KEY), false, false};
    request.requestNumber = multiResponseId;

    return request;
}