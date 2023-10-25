#pragma once

#include "persistence/OperationLog.h"
#include "cluster/Cluster.h"
#include "operators/OperationOptions.h"
#include "operators/operations/syncoplog/OnGoingSyncOplogsStore.h"
#include "db/MemDbStores.h"
#include "logging/Logger.h"

struct OperatorDependencies {
    onGoingSyncOplogs_t onGoingSyncOplogs;
    configuration_t configuration;
    operationLog_t operationLog;
    memDbStores_t memDbStores;
    cluster_t cluster;
    logger_t logger;

    std::function<void(const std::vector<OperationBody>&, const OperationOptions&)> operatorsDispatcher;
    std::function<Response(const OperationBody&, const OperationOptions&)> operatorDispatcher;
};