#include "cluster/clusterdb/ClusterDbValueChanged.h"

static bool isCreate(const std::string& action);

std::string ClusterDbChangeTypes::toString(ClusterDbChangeType clusterDbChangeType) {
    return clusterDbChangeType == ClusterDbChangeType::DELETED ? "DELETED" : "PUT";
}

ClusterDbValueChanged ClusterDbValueChanged::fromEtcdResponse(const etcd::Response& response)  {
    auto actionType = isCreate(response.action()) ? ClusterDbChangeType::PUT : ClusterDbChangeType::DELETED;
    auto value = response.value().as_string() == "" ?
            nlohmann::json::parse(response.prev_value().as_string()) :
            nlohmann::json::parse(response.value().as_string());

    return ClusterDbValueChanged{.changeType = actionType, .value = value};
}

static bool isCreate(const std::string& action) {
    return action == "set" || action == "create";
}