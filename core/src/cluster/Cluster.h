#pragma once

#include "shared.h"

#include "cluster/partitions/Partitions.h"
#include "cluster/NodeState.h"
#include "cluster/othernodes/ClusterNodes.h"
#include "cluster/clusterdb/ClusterDb.h"
#include "cluster/othernodes/NodePartitionOptions.h"

#include "utils/clock/LamportClock.h"
#include "utils/strings/StringUtils.h"

#include "operators/operations/syncoplog/SyncOplogReceiverIterator.h"
#include "operators/operations/syncoplog/OnGoingSyncOplogsStore.h"
#include "messages/request/RequestBuilder.h"

#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"

#include "persistence/serializers/OperationLogDeserializer.h"
#include "logging/Logger.h"
#include "db/MemDbStores.h"
#include "operators/OperatorNumbers.h"


class Cluster {
private:
    onGoingSyncOplogs_t onGoingMultipleResponsesStore;
    configuration_t configuration;
    clusterNodes_t clusterNodes;
    memDbStores_t memDbStores;
    partitions_t partitions;
    clusterdb_t clusterDb;
    node_t selfNode;
    logger_t logger;

    friend class PartitionNeighborsNodesSetter;
    friend class PartitionClusterNodeChangeHandler;
    friend class SimpleClusterNodeChangeHandler;
    friend class NewNodePartitionChangeHandler;
    friend class PartitionsClusterNodeSetup;
    friend class DeletionNodeChangeHandler;
    friend class SimpleClusterNodeSetup;
    friend class ClusterNodeSetup;
    friend class ClusterTest;

public:
    Cluster(configuration_t configuration);

    Cluster(logger_t logger, configuration_t configuration, onGoingSyncOplogs_t onGoingMultipleResponsesStore,
            memDbStores_t memDbStores, clusterdb_t clusterDb);

    auto setBooting() -> void;

    auto setRunning() -> void;

    auto syncOplog(uint64_t lastTimestampProcessedFromOpLog, const NodePartitionOptions options = {}) -> iterator_t<std::vector<uint8_t>>;

    auto getPartitionObject() -> partitions_t;

    auto getPartitionIdByKey(SimpleString<memDbDataLength_t> key) -> uint32_t;

    auto getNodesPerPartition() -> uint32_t;

    virtual auto broadcast(const OperationBody& operation) -> void;

    virtual auto getNodeState() -> NodeState;

    auto getNodeId() -> memdbNodeId_t;

    auto watchForChangesInNodesClusterDb(std::function<void(node_t nodeChanged, ClusterDbChangeType changeType)> onChangeCallback) -> void;
};

using cluster_t = std::shared_ptr<Cluster>;