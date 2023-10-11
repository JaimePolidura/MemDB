#pragma once

#include "shared.h"

#include "cluster/clustermanager/ClusterManagerService.h"
#include "cluster/partitions/Partitions.h"
#include "cluster/NodeState.h"
#include "cluster/othernodes/ClusterNodes.h"
#include "cluster/clusterdb/ClusterDb.h"
#include "cluster/othernodes/NodeGroupOptions.h"

#include "utils/clock/LamportClock.h"
#include "utils/strings/StringUtils.h"

#include "messages/multi/MultiResponseReceiverIterator.h"
#include "messages/multi/OnGoingMultipleResponsesStore.h"

#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"

#include "persistence/serializers/OperationLogDeserializer.h"
#include "logging/Logger.h"
#include "db/MemDbStores.h"

class Cluster {
private:
    onGoingMultipleResponsesStore_t onGoingMultipleResponsesStore;
    configuration_t configuration;
    clusterNodes_t clusterNodes;
    clusterManagerService_t clusterManager;
    memDbStores_t memDbStores;
    partitions_t partitions;
    clusterdb_t clusterDb;
    node_t selfNode;
    logger_t logger;

    friend class PartitionNeighborsNodesGroupSetter;
    friend class PartitionClusterNodeChangeHandler;
    friend class SimpleClusterNodeChangeHandler;
    friend class NewNodePartitionChangeHandler;
    friend class PartitionsClusterNodeSetup;
    friend class DeletionNodeChangeHandler;
    friend class SimpleClusterNodeSetup;
    friend class ClusterNodeSetup;
    friend class ClusterTest;

public:
    Cluster(): partitions(std::make_shared<Partitions>()), clusterNodes(std::make_shared<ClusterNodes>()) {}

    Cluster(logger_t logger, configuration_t configuration, onGoingMultipleResponsesStore_t onGoingMultipleResponsesStore, memDbStores_t memDbStores);

    auto setBooting() -> void;

    auto setRunning() -> void;

    auto syncOplog(uint64_t lastTimestampProcessedFromOpLog, const NodeGroupOptions options = {}) -> multiResponseReceiverIterator_t;

    auto getPartitionObject() -> partitions_t;

    auto getPartitionIdByKey(SimpleString<memDbDataLength_t> key) -> uint32_t;

    auto getNodesPerPartition() -> uint32_t;

    virtual auto broadcast(const OperationBody& operation) -> void;

    virtual auto getNodeState() -> NodeState;

    auto getNodeId() -> memdbNodeId_t;

    auto watchForChangesInNodesClusterDb(std::function<void(node_t nodeChanged, ClusterDbChangeType changeType)> onChangeCallback) -> void;

private:
    auto createSyncOplogRequestInitMultiResponse(uint64_t timestamp, uint32_t selfOplogId, memdbNodeId_t nodeIdToSendRequest) -> Request;

    auto createNextFragmentMultiResponseRequest(uint64_t multiResponseId) -> Request;
};

using cluster_t = std::shared_ptr<Cluster>;