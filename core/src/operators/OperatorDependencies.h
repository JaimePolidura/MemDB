#pragma once

#include "persistence/OperationLog.h"
#include "utils/datastructures/map/Map.h"
#include "cluster/Cluster.h"
#include "operators/OperationOptions.h"
#include "operators/multi/MultipleResponses.h"

struct OperatorDependencies {
    multipleResponses_t multipleResponses;
    configuration_t configuration;
    operationLog_t operationLog;
    memDbDataStore_t dbStore;
    cluster_t cluster;

    std::function<void(const std::vector<OperationBody>&, const OperationOptions&)> operatorsDispatcher;
    std::function<Response(const OperationBody&, const OperationOptions&)> operatorDispatcher;
};