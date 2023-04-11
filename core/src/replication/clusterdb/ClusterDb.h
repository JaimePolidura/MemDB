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
    logger_t logger;

public:
    ClusterDb(configuration_t configuration, logger_t logger) : configuration(configuration), logger(logger) {}

    auto watch(const std::string& directory, std::function<void(ClusterDbValueChanged)> onChange) -> void {
        etcd::Client client{configuration->get(ConfigurationKeys::ETCD_ADDRESSES)};

        client.watch(directory, true).then([this, directory, onChange](pplx::task<etcd::Response> responseTaskPplx){
            onChange(ClusterDbValueChanged::fromEtcdResponse(responseTaskPplx.get()));

            watch(directory, onChange);
        });
    }

    auto set(const std::string& nodeId, const Node& node) -> void {
        etcd::Client client{configuration->get(ConfigurationKeys::ETCD_ADDRESSES)};

        client.put("/nodes/" + nodeId, Node::toJson(node));
    }
};

using clusterdb_t = std::shared_ptr<ClusterDb>;