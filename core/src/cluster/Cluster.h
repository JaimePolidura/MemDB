#pragma once

#include "shared.h"
#include "changehandler/ClusterDbNodeChangeHandler.h"

#include "cluster/othernodes/NodePartitionOptions.h"
#include "cluster/othernodes/MultipleResponses.h"
#include "cluster/GetClusterConfigResponse.h"
#include "cluster/othernodes/ClusterNodes.h"
#include "cluster/partitions/Partitions.h"
#include "cluster/NodeState.h"

#include "operators/operations/syncoplog/SyncOplogReceiverIterator.h"
#include "operators/operations/syncoplog/OnGoingSyncOplogsStore.h"

#include "config/Configuration.h"

#include "logging/Logger.h"
#include "db/MemDbStores.h"

class Cluster {
public:
    Cluster(configuration_t configuration);

    Cluster(logger_t logger, configuration_t configuration, onGoingSyncOplogs_t onGoingMultipleResponsesStore,
            memDbStores_t memDbStores);

    auto setBooting() -> void;

    auto setRunning() -> void;

    auto syncOplog(uint64_t lastTimestampProcessedFromOpLog, const NodePartitionOptions options) -> iterator_t<std::result<std::vector<uint8_t>>>;

    auto fixOplogSegment(uint32_t selfOplogId, uint64_t minTimestamp, uint64_t maxTimestamp) -> std::result<Response>;

    auto getPartitionObject() -> partitions_t;
    
    auto getClusterConfig() -> std::result<GetClusterConfigResponse>;

    auto announceJoin() -> void;

    auto announceLeave() -> void;

    auto getPartitionIdByKey(SimpleString<memDbDataLength_t> key) -> uint32_t;

    auto getNodesPerPartition() -> uint32_t;

    auto getMaxPartitionSize() -> uint32_t;

    auto broadcastAndWait(const NodePartitionOptions options, const OperationBody& operation) -> multipleResponses_t;

    virtual auto broadcast(const NodePartitionOptions options, const OperationBody& operation) -> void;

    virtual auto getNodeState() -> NodeState;

    auto getNodeId() -> memdbNodeId_t;

private:
    clusterDbNodeChangeHandler_t clusterChangeHandler;
    onGoingSyncOplogs_t onGoingMultipleResponsesStore;
    configuration_t configuration;
    clusterNodes_t clusterNodes;
    memDbStores_t memDbStores;
    partitions_t partitions;
    node_t selfNode;
    logger_t logger;

    friend class PartitionClusterNodeChangeHandler;
    friend class SimpleClusterNodeChangeHandler;
    friend class PartitionNeighborsNodesSetter;
    friend class NewNodePartitionChangeHandler;
    friend class LeaveClusterAnnounceOperator;
    friend class JoinClusterAnnounceOperator;
    friend class PartitionsClusterNodeSetup;
    friend class DeletionNodeChangeHandler;
    friend class GetClusterConfigOperator;
    friend class DoLeaveClusterOperator;
    friend class SimpleClusterNodeSetup;
    friend class ClusterNodeSetup;
    friend class ClusterTest;

    std::vector<RingEntry> getRingEntriesFromGetClusterConfig(uint32_t nNodesInCluster, int& offset, Response response);
    std::vector<node_t> getNodesFromGetClusterConfig(uint32_t nNodesInCluster, int& offset, Response response);
};

using cluster_t = std::shared_ptr<Cluster>;