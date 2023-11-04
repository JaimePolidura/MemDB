#pragma once

#include "operators/operations/cas/OnGoingPaxosRounds.h"
#include "operators/DelayedOperationsBuffer.h"
#include "operators/OperatorDependencies.h"
#include "operators/OperatorRegistry.h"

#include "operators/Operator.h"

#include "operators/operations/syncoplog/OnGoingSyncOplogsStore.h"
#include "messages/response/ErrorCode.h"

#include "db/MemDbStores.h"

#include "persistence/SingleOperationLog.h"
#include "utils/clock/LamportClock.h"
#include "cluster/Cluster.h"

class OperatorDispatcher {
public: //Need it for mocking it
    operatorRegistry_t operatorRegistry;
private:
    DelayedOperationsBuffer delayedOperationsBuffer{};
    onGoingPaxosRounds_t onGoingPaxosRounds;
    onGoingSyncOplogs_t onGoingSyncOplogs;
    operationLog_t operationLog;
    configuration_t configuration;
    cluster_t cluster;
    lamportClock_t clock;
    memDbStores_t memDbStores;
    logger_t logger;

    OperatorDependencies dependencies;

public:
    OperatorDispatcher(memDbStores_t memDbStores, lamportClock_t clock, cluster_t cluster, configuration_t configuration,
                       logger_t logger, operationLog_t operationLog, onGoingSyncOplogs_t onGoingSyncOplogs);

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

    inline int getPartitionIdByKey(const SimpleString<memDbDataLength_t>& key);

    bool isInReplicationMode();

    bool isInPartitionMode();

    bool canHoldKey(arg_t arg);

    bool canAcceptRequest();

    bool canExecuteRequest();
};

using operatorDispatcher_t = std::shared_ptr<OperatorDispatcher>;