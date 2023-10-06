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

#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"

#include "logging/Logger.h"
#include "persistence/serializers/OperationLogDeserializer.h"

class Cluster {
private:
    configuration_t configuration;
    clusterNodes_t clusterNodes;
    clusterManagerService_t clusterManager;
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

    Cluster(logger_t logger, configuration_t configuration);

    auto setBooting() -> void;

    auto setRunning() -> void;

    auto getUnsyncedOplog(uint64_t lastTimestampProcessedFromOpLog, const NodeGroupOptions options = {}) -> std::vector<OperationBody>;

    auto getPartitionObject() -> partitions_t;

    virtual auto broadcast(const OperationBody& operation) -> void;

    virtual auto getNodeState() -> NodeState;

    auto watchForChangesInNodesClusterDb(std::function<void(node_t nodeChanged, ClusterDbChangeType changeType)> onChangeCallback) -> void;

private:
    auto createSyncOplogRequest(uint64_t timestamp, uint32_t selfOplogId, memdbNodeId_t nodeIdToSendRequest) -> Request;
};

using cluster_t = std::shared_ptr<Cluster>;