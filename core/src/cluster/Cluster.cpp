#include "cluster/Cluster.h"

Cluster::Cluster(configuration_t configuration): configuration(configuration) {}

Cluster::Cluster(logger_t logger, configuration_t configuration, onGoingSyncOplogs_t onGoingMultipleResponsesStore, memDbStores_t memDbStores, clusterdb_t clusterDb) :
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

auto Cluster::fixOplogSegment(uint32_t selfOplogId, uint64_t minTimestamp, uint64_t maxTimestamp) -> std::optional<Response> {
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
        node->setLogger(this->logger);
        bool selfNodeChanged = node->nodeId == this->configuration->get<memdbNodeId_t>(ConfigurationKeys::NODE_ID);

        if (!selfNodeChanged || nodeChangedEvent.changeType == ClusterDbChangeType::DELETED) {
            onChangeCallback(node, nodeChangedEvent.changeType);
        }
    });
}