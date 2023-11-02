#pragma once

#include "persistence/OperationLog.h"
#include "cluster/Cluster.h"
#include "operators/OperationOptions.h"
#include "operators/operations/syncoplog/OnGoingSyncOplogsStore.h"
#include "operators/operations/cas/OnGoingPaxosRounds.h"
#include "db/MemDbStores.h"
#include "logging/Logger.h"
#include "persistence/segments/OplogIndexSegment.h"
#include "utils/clock/LamportClock.h"

struct OperatorDependencies {
    onGoingPaxosRounds_t onGoingPaxosRounds;
    onGoingSyncOplogs_t onGoingSyncOplogs;
    oplogIndexSegment_t oplogIndexSegment;
    configuration_t configuration;
    operationLog_t operationLog;
    memDbStores_t memDbStores;
    lamportClock_t clock;
    cluster_t cluster;
    logger_t logger;

    std::function<void(const std::vector<OperationBody>&, const OperationOptions&)> operatorsDispatcher;
    std::function<Response(const OperationBody&, const OperationOptions&)> operatorDispatcher;
};