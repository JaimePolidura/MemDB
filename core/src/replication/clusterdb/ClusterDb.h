#pragma once

#include "shared.h"

#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "replication/clusterdb/ClusterDbValueChanged.h"
#include "utils/strings/StringUtils.h"
#include "utils/net/DNSUtils.h"
#include "logging/Logger.h"

//TOOD Improve, move etcd client to class level
class ClusterDb {
private:
    configuration_t configuration;
    etcd::Client client;
    logger_t logger;

public:
    ClusterDb(configuration_t configuration, logger_t logger) : client(configuration->get(ConfigurationKeys::ETCD_ADDRESSES)), configuration(configuration), logger(logger) {}

    auto watchNodeChanges(std::function<void(ClusterDbValueChanged)> onChange) -> void {
        etcd::Watcher(client, "/nodes", [onChange](etcd::Response response){
            onChange(ClusterDbValueChanged::fromEtcdResponse(response));
        }, true);
    }

    auto setNode(const std::string& nodeId, const Node& node) -> void {
        client.put("/nodes/" + nodeId, Node::toJson(node));
    }
};

using clusterdb_t = std::shared_ptr<ClusterDb>;