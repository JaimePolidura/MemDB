#pragma once

#include "shared.h"

#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "cluster/clusterdb/ClusterDbValueChanged.h"
#include "utils/strings/StringUtils.h"
#include "utils/net/DNSUtils.h"
#include "logging/Logger.h"
#include "cluster/partitions/RingEntry.h"
#include "cluster/Node.h"

class ClusterDb {
private:
    std::unique_ptr<etcd::Watcher> nodeChangesWatcher;
    configuration_t configuration;
    etcd::Client client;
    logger_t logger;

public:
    ClusterDb(configuration_t configuration, logger_t logger) : client(configuration->get(ConfigurationKeys::MEMDB_CORE_ETCD_ADDRESSES)),
        configuration(configuration), logger(logger) {}

    void watchNodeChanges(std::function<void(ClusterDbValueChanged)> onChange);

    void setNode(memdbNodeId_t nodeId, node_t node);

    RingEntry getRingEntryByNodeId(memdbNodeId_t nodeId);
};

using clusterdb_t = std::shared_ptr<ClusterDb>;