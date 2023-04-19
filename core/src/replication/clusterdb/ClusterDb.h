#pragma once

#include "shared.h"

#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "replication/clusterdb/ClusterDbValueChanged.h"
#include "utils/strings/StringUtils.h"
#include "utils/net/DNSUtils.h"
#include "logging/Logger.h"

class ClusterDb {
private:
    std::unique_ptr<etcd::Watcher> nodeChangesWatcher;
    configuration_t configuration;
    etcd::Client client;
    logger_t logger;

public:
    ClusterDb(configuration_t configuration, logger_t logger) : client(configuration->get(ConfigurationKeys::MEMDB_CORE_ETCD_ADDRESSES)),
        configuration(configuration), logger(logger) {}

    auto watchNodeChanges(std::function<void(ClusterDbValueChanged)> onChange) -> void {
        this->nodeChangesWatcher = std::make_unique<etcd::Watcher>(client, "/nodes", [onChange](etcd::Response response){
            onChange(ClusterDbValueChanged::fromEtcdResponse(response));
        }, true);
    }

    auto setNode(memdbNodeId_t nodeId, node_t node) -> void {
        client.put("/nodes/" + std::to_string(nodeId), Node::toJson(node));
    }
};

using clusterdb_t = std::shared_ptr<ClusterDb>;