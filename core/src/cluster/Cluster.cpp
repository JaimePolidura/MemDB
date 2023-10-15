#include "cluster/Cluster.h"

Cluster::Cluster(configuration_t configuration): configuration(configuration) {}

Cluster::Cluster(logger_t logger, configuration_t configuration, onGoingMultipleResponsesStore_t onGoingMultipleResponsesStore, memDbStores_t memDbStores, clusterdb_t clusterDb) :
    configuration(configuration),
    memDbStores(memDbStores),
    clusterDb(clusterDb),
    onGoingMultipleResponsesStore(onGoingMultipleResponsesStore),
    clusterNodes(std::make_shared<ClusterNodes>(configuration, logger)),
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
    return this->configuration->get<memdbNodeId_t>(ConfigurationKeys::NODE_ID);
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

auto Cluster::watchForChangesInNodesClusterDb(std::function<void(node_t nodeChanged, ClusterDbChangeType changeType)> onChangeCallback) -> void {
    this->clusterDb->watchNodeChanges([this, onChangeCallback](ClusterDbValueChanged nodeChangedEvent) {
        node_t node = Node::fromJson(nodeChangedEvent.value);
        bool selfNodeChanged = node->nodeId == this->configuration->get<memdbNodeId_t>(ConfigurationKeys::NODE_ID);

        if (!selfNodeChanged) {
            onChangeCallback(node, nodeChangedEvent.changeType);
        }
        if (selfNodeChanged && node->state == NodeState::SHUTDOWN) { //Reload
            this->setRunning();
        }
    });
}

auto Cluster::createSyncOplogRequestInitMultiResponse(uint64_t timestamp, uint32_t selfOplogId, memdbNodeId_t nodeIdToSendRequest) -> Request {
    uint32_t part1 = timestamp >> 32;
    uint32_t part2 = (uint32_t) timestamp & 0xFFFFFFFF;
    uint32_t nodeOplogId = this->configuration->getBoolean(ConfigurationKeys::USE_PARTITIONS) ?
                           (this->partitions->isClockwiseNeighbor(nodeIdToSendRequest) ?
                            selfOplogId + this->partitions->getDistanceClockwise(nodeIdToSendRequest) :
                            selfOplogId - this->partitions->getDistanceCounterClockwise(nodeIdToSendRequest)) : 0;

    return RequestBuilder::builder()
            .authKey(this->configuration->get(ConfigurationKeys::AUTH_NODE_KEY))
            ->args({
                SimpleString<memDbDataLength_t>::fromNumber(OperatorNumbers::SYNC_OPLOG),
                SimpleString<memDbDataLength_t>::fromNumber(part1),
                SimpleString<memDbDataLength_t>::fromNumber(part2),
                SimpleString<memDbDataLength_t>::fromNumber(nodeOplogId)
            })
            ->selfNode(this->selfNode->nodeId)
            ->operatorNumber(OperatorNumbers::INIT_MULTI)
            ->requestNumber(this->onGoingMultipleResponsesStore->nextMultiResponseId()) // Used as multi-response Id
            ->build();
}

auto Cluster::createNextFragmentMultiResponseRequest(uint64_t multiResponseId) -> Request {
    return RequestBuilder::builder()
            .authKey(this->configuration->get(ConfigurationKeys::AUTH_NODE_KEY))
            ->operatorNumber(OperatorNumbers::NEXT_FRAGMENT)
            ->requestNumber(multiResponseId)
            ->build();
}