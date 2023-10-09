#pragma once

#include "persistence/OperationLog.h"
#include "utils/datastructures/map/Map.h"
#include "cluster/Cluster.h"
#include "operators/OperationOptions.h"
#include "messages/multi/OnGoingMultipleResponsesStore.h"

struct OperatorDependencies {
    onGoingMultipleResponsesStore_t multipleResponses;
    configuration_t configuration;
    operationLog_t operationLog;
    memDbDataStore_t dbStore;
    cluster_t cluster;

    std::function<void(const std::vector<OperationBody>&, const OperationOptions&)> operatorsDispatcher;
    std::function<Response(const OperationBody&, const OperationOptions&)> operatorDispatcher;
    std::function<multipleResponseSenderIterator_t(const OperationBody&, uint8_t)> getMultiResponseSenderIterator;
};