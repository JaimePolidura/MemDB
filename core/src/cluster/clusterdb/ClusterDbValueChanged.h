#pragma once

#include "shared.h"

enum ClusterDbChangeType {
    DELETED, PUT
};

struct ClusterDbChangeTypes {
    static std::string toString(ClusterDbChangeType clusterDbChangeType);
};

struct ClusterDbValueChanged {
    ClusterDbChangeType changeType;
    nlohmann::json value;
    
    static ClusterDbValueChanged fromEtcdResponse(const etcd::Response& response);
};