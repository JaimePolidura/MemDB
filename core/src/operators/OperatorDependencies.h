#pragma once

#include "persistence/OperationLog.h"
#include "cluster/Cluster.h"
#include "operators/OperationOptions.h"
#include "messages/multi/OnGoingMultipleResponsesStore.h"
#include "db/MemDbStores.h"

struct OperatorDependencies {
    onGoingMultipleResponsesStore_t multipleResponses;
    configuration_t configuration;
    operationLog_t operationLog;
    memDbStores_t memDbStores;
    cluster_t cluster;

    std::function<void(const std::vector<OperationBody>&, const OperationOptions&)> operatorsDispatcher;
    std::function<Response(const OperationBody&, const OperationOptions&)> operatorDispatcher;
    std::function<iterator_t<std::vector<uint8_t>>(const OperationBody&, uint8_t)> getMultiResponseSenderIterator;
};