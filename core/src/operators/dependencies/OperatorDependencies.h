#pragma once

#include "persistence/oplog/OperationLog.h"
#include "utils/datastructures/map/Map.h"
#include "cluster/Cluster.h"
#include "operators/OperationOptions.h"

struct OperatorDependencies {
    memDbDataStore_t dbStore;
    operationLog_t operationLog;
    cluster_t cluster;
    std::function<Response(const OperationBody&, const OperationOptions&)> operatorDispatcher;
};