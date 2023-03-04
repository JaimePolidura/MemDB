#pragma once

#include <etcd/Client.hpp>
#include <nlohmann/json.hpp>

#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "ClusterDbValueChanged.h"

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
};

using clusterdb_t = std::shared_ptr<ClusterDb>;