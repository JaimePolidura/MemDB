#pragma once

#include "shared.h"

enum ClusterDbChangeType {
    DELETED, PUT
};

struct ClusterDbChangeTypes {
    static std::string toString(ClusterDbChangeType clusterDbChangeType) {
        return clusterDbChangeType == ClusterDbChangeType::DELETED ? "DELETED" : "PUT";
    }
};

struct ClusterDbValueChanged {
    ClusterDbChangeType changeType;
    nlohmann::json value;

    static auto fromEtcdResponse(const etcd::Response& response) -> ClusterDbValueChanged {
        auto actionType = response.action() == "set" ? ClusterDbChangeType::PUT : ClusterDbChangeType::DELETED;
        auto value = response.value().as_string() == "" ?
                nlohmann::json::parse(response.prev_value().as_string()) :
                nlohmann::json::parse(response.value().as_string());

        return ClusterDbValueChanged{.changeType = actionType, .value = value};
    };
};