#pragma once

#include "operators/Operator.h"
#include "operators/OperatorRegistry.h"
#include "operators/DelayedOperationsBuffer.h"
#include "operators/OperatorDependencies.h"

#include "messages/multi/OnGoingMultipleResponsesStore.h"
#include "messages/response/ErrorCode.h"

#include "db/MemDbStores.h"

#include "persistence/SingleOperationLog.h"
#include "utils/clock/LamportClock.h"
#include "cluster/Cluster.h"

class OperatorDispatcher {
public: //Need it for mocking it
    operatorRegistry_t operatorRegistry;
private:
    delayedOperationsBuffer_t delayedOperationsBuffer;
    onGoingMultipleResponsesStore_t multipleResponses;
    operationLog_t operationLog;
    configuration_t configuration;
    cluster_t cluster;
    lamportClock_t clock;
    memDbStores_t memDbStores;
    logger_t logger;

    OperatorDependencies dependencies;

public:
    OperatorDispatcher(memDbStores_t memDbStores, lamportClock_t clock, cluster_t cluster, configuration_t configuration,
                       logger_t logger, operationLog_t operationLog, onGoingMultipleResponsesStore_t multipleResponses);

    Response dispatch(const Request& request);

    void executeOperations(std::shared_ptr<Operator> operatorToExecute,
                          const std::vector<OperationBody>& operations,
                          const OperationOptions& options);

    Response executeOperation(std::shared_ptr<Operator> operatorToExecute,
                              OperationBody& operation,
                              const OperationOptions& options);

    void applyDelayedOperationsBuffer();

private:
    Response dispatch_no_applyDelayedOperationsBuffer(const Request& request);

    OperatorDependencies getDependencies();

    bool isAuthorizedToExecute(std::shared_ptr<Operator> operatorToExecute, AuthenticationType authenticationOfUser);

    uint64_t updateClock(LamportClock::UpdateClockStrategy updateStrategy, uint64_t newValue);

    bool isInReplicationMode();

    bool isInPartitionMode();

    bool canHoldKey(arg_t arg);

    bool canAcceptRequest();

    bool canExecuteRequest();
};

using operatorDispatcher_t = std::shared_ptr<OperatorDispatcher>;