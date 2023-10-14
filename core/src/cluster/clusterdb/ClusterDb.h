#pragma once

#include "cluster/clusterdb/responses/PartitionConfigurationResponse.h"
#include "cluster/clusterdb/responses/GetRingNeighborsResponse.h"
#include "cluster/clusterdb/responses/GetRingInfoResponse.h"
#include "cluster/clusterdb/responses/AllNodesResponse.h"
#include "cluster/clusterdb/ClusterDbValueChanged.h"
#include "cluster/partitions/RingEntry.h"
#include "cluster/Node.h"

#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"

#include "utils/strings/StringUtils.h"
#include "utils/net/DNSUtils.h"

#include "logging/Logger.h"
#include "shared.h"

class ClusterDb {
private:
    std::unique_ptr<etcd::Watcher> nodeChangesWatcher;
    configuration_t configuration;
    etcd::Client client;
    logger_t logger;

public:
    ClusterDb(configuration_t configuration, logger_t logger);

    void watchNodeChanges(std::function<void(ClusterDbValueChanged)> onChange);

    void setNode(memdbNodeId_t nodeId, node_t node);

    node_t getByNodeId(memdbNodeId_t nodeId);

    AllNodesResponse getAllNodes();

    PartitionConfigurationResponse getPartitionsConfiguration();

    RingEntry getRingEntryByNodeId(memdbNodeId_t nodeId);

    std::vector<RingEntry> getRingEntries();
};

using clusterdb_t = std::shared_ptr<ClusterDb>;