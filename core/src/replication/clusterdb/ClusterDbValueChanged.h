#pragma once

#include "shared.h"

enum ClusterDbChangeType {
    DELETED, PUT
};

struct ClusterDbValueChanged {
    ClusterDbChangeType changeType;
    nlohmann::json value;

    static auto fromEtcdResponse(const etcd::Response& response) -> ClusterDbValueChanged {
        auto actionType = response.action() == "PUT" ? ClusterDbChangeType::PUT : ClusterDbChangeType::DELETED;
        auto value = nlohmann::json::parse(response.value().as_string());

        return ClusterDbValueChanged{.changeType = actionType, .value = value};
    };
};