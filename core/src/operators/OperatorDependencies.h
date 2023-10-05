#pragma once

#include "persistence/oplog/OperationLog.h"
#include "utils/datastructures/map/Map.h"
#include "cluster/Cluster.h"
#include "operators/OperationOptions.h"

struct OperatorDependencies {
    configuration_t configuration;
    memDbDataStore_t dbStore;
    operationLog_t operationLog;
    cluster_t cluster;

    std::function<void(const std::vector<OperationBody>&, const OperationOptions&)> operatorsDispatcher;
    std::function<Response(const OperationBody&, const OperationOptions&)> operatorDispatcher;
};