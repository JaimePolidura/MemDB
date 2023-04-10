#pragma once

#include "shared.h"

#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "replication/clusterdb/ClusterDbValueChanged.h"

class ClusterDb {
private:
    configuration_t configuration;
    etcd::Client client;

public:
    ClusterDb(configuration_t configuration) : configuration(configuration), client(configuration->get(ConfigurationKeys::ETCD_ADDRESSES)) {}

    auto watch(const std::string& directory, std::function<void(ClusterDbValueChanged)> onChange) -> void {
        this->client.watch(directory, true).then([this, directory, onChange](pplx::task<etcd::Response> responseTaskPplx){
            onChange(ClusterDbValueChanged::fromEtcdResponse(responseTaskPplx.get()));

            watch(directory, onChange);
        });
    }

    auto set(const std::string& nodeId, const Node& node) -> void {
        this->client.put("/nodes/" + nodeId, Node::toJson(node)).get();
    }
};

using clusterdb_t = std::shared_ptr<ClusterDb>;