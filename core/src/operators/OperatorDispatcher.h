#pragma once

#include "operators/Operator.h"
#include "operators/OperatorRegistry.h"
#include "operators/DbOperatorExecutor.h"
#include "operators/dependencies/OperatorDependencies.h"
#include "operators/dependencies/OperatorDependency.h"

#include "messages/response/ErrorCode.h"
#include "persistence/oplog/SingleOperationLog.h"
#include "utils/clock/LamportClock.h"
#include "cluster/Cluster.h"
#include "DelayedOperationsBuffer.h"

class OperatorDispatcher {
public: //Need it for mocking it
    operatorRegistry_t operatorRegistry;
private:
    delayedOperationsBuffer_t delayedOperationsBuffer;
    operationLog_t operationLog;
    configuration_t configuration;
    cluster_t cluster;
    lamportClock_t clock;
    memDbDataStore_t db;
    logger_t logger;

public:
    OperatorDispatcher(memDbDataStore_t dbCons, lamportClock_t clock, cluster_t cluster, configuration_t configuration,
                       logger_t logger, operationLog_t operationLog);

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

    OperatorDependencies getDependencies(std::shared_ptr<Operator> operatorToGetDependencies);

    void getDependency(OperatorDependency dependency, OperatorDependencies * operatorDependencies);

    bool isAuthorizedToExecute(std::shared_ptr<Operator> operatorToExecute, AuthenticationType authenticationOfUser);

    uint64_t updateClock(LamportClock::UpdateClockStrategy updateStrategy, uint64_t newValue);

    bool isInReplicationMode();

    bool isInPartitionMode();

    bool canHoldKey(arg_t arg);

    bool canAcceptRequest();

    bool canExecuteRequest();
};

using operatorDispatcher_t = std::shared_ptr<OperatorDispatcher>;