#include "operators/OperatorDispatcher.h"

OperatorDispatcher::OperatorDispatcher(memDbStores_t memDbStores, lamportClock_t clock, cluster_t cluster, configuration_t configuration,
                                       logger_t logger, operationLog_t operationLog, onGoingSyncOplogs_t onGoingSyncOplogs):
        memDbStores(memDbStores),
        operationLog(operationLog),
        onGoingSyncOplogs(onGoingSyncOplogs),
        clock(clock),
        logger(logger),
        cluster(cluster),
        configuration(configuration),
        delayedOperationsBuffer(configuration) {
    this->dependencies = this->getDependencies();
}

Response OperatorDispatcher::dispatch(const Request& request) {
    if(canExecuteRequest()){
        applyDelayedOperationsBuffer();
    }

    return this->dispatch_no_applyDelayedOperationsBuffer(request);
}

Response OperatorDispatcher::dispatch_no_applyDelayedOperationsBuffer(const Request &request) {
    operator_t operatorToExecute = this->operatorRegistry->get(request.operation.operatorNumber);

    if(operatorToExecute == nullptr){
        return Response::error(ErrorCode::UNKNOWN_OPERATOR);
    }
    if(!isAuthorizedToExecute(operatorToExecute, request.authenticationType)) {
        return Response::error(ErrorCode::NOT_AUTHORIZED);
    }

    bool writeDbRequestFromNode = operatorToExecute->desc().type == OperatorType::DB_STORE_WRITE && request.authenticationType == AuthenticationType::NODE;
    bool writeDbRequest = operatorToExecute->desc().type == OperatorType::DB_STORE_WRITE;
    bool readDbRequest = operatorToExecute->desc().type == OperatorType::DB_STORE_READ;

    OperationOptions options = {
            .checkTimestamps = writeDbRequestFromNode,
            .updateClockStrategy = writeDbRequestFromNode ? LamportClock::UpdateClockStrategy::SET_MAX : LamportClock::UpdateClockStrategy::TICK,
            .requestNumber = request.requestNumber,
    };

    if(isInReplicationMode() && (!canAcceptRequest() || (readDbRequest && !canExecuteRequest()))) {
        return Response::error(ErrorCode::INVALID_NODE_STATE);
    }
    if(isInReplicationMode() && isInPartitionMode() && writeDbRequest && !canHoldKey(request.operation.args->at(0))) {
        return Response::error(ErrorCode::INVALID_PARTITION);
    }
    if(isInReplicationMode() && writeDbRequest && canAcceptRequest() && !canExecuteRequest()){
        this->delayedOperationsBuffer.add(request);
        return Response::success();
    }
    if(operatorToExecute->desc().type == OperatorType::DB_STORE_WRITE ||
            operatorToExecute->desc().type == OperatorType::DB_STORE_READ ||
            operatorToExecute->desc().type == OperatorType::DB_STORE_CONDITIONAL_WRITE) {
        options.partitionId = this->cluster->getPartitionIdByKey(request.operation.getArg(0));
    }

    OperationBody operationBody = request.operation;

    return this->executeOperation(operatorToExecute, operationBody, options);
}

void OperatorDispatcher::executeOperations(std::shared_ptr<Operator> operatorToExecute,
                       const std::vector<OperationBody>& operations,
                       const OperationOptions& options) {

    for (const OperationBody& operation: operations){
        executeOperation(operatorToExecute, const_cast<OperationBody&>(operation), options);
    }
}

Response OperatorDispatcher::executeOperation(std::shared_ptr<Operator> operatorToExecute,
                          OperationBody& operation,
                          const OperationOptions& options) {
    Response result = operatorToExecute->operate(operation, options, this->dependencies);

    if(operatorToExecute->desc().type == DB_STORE_WRITE && result.isSuccessful && !options.onlyExecute) {
        if(options.fromClient()) {
            operation.timestamp = result.timestamp.counter;
        }

        if(!options.dontSaveInOperationLog){
            this->operationLog->add(options.partitionId, operation);
        }

        if(isInReplicationMode() && options.fromClient() && !options.dontBroadcastToCluster){
            this->cluster->broadcast({.partitionId = this->getPartitionIdByKey(operation.getArg(0))}, operation);
            this->logger->debugInfo("Broadcasting request for operator {0} of key {1} with timestamp ({2}, {3})",
                                    operatorToExecute->desc().name, operation.getArg(0).toString(), operation.timestamp, cluster->getNodeId());
        }
    }

    return result;
}

void OperatorDispatcher::applyDelayedOperationsBuffer() {
    Iterator<Request>& iterator = dynamic_cast<Iterator<Request>&>(this->delayedOperationsBuffer.iterator());

    while(iterator.hasNext()) {
        Request operation = iterator.next();
        this->dispatch_no_applyDelayedOperationsBuffer(operation);
    }
}

OperatorDependencies OperatorDispatcher::getDependencies() {
    OperatorDependencies dependenciesToReturn{};

    dependenciesToReturn.onGoingPaxosRounds = this->onGoingPaxosRounds;
    dependenciesToReturn.onGoingSyncOplogs = this->onGoingSyncOplogs;
    dependenciesToReturn.configuration = this->configuration;
    dependenciesToReturn.operationLog = this->operationLog;
    dependenciesToReturn.cluster = this->cluster;
    dependenciesToReturn.logger = this->logger;
    dependenciesToReturn.clock = this->clock;
    dependenciesToReturn.memDbStores = this->memDbStores;
    dependenciesToReturn.operatorDispatcher = [this](const OperationBody& op, const OperationOptions& options) -> Response {
        return this->executeOperation(this->operatorRegistry->get(op.operatorNumber), const_cast<OperationBody&>(op), options);
    };
    dependenciesToReturn.operatorsDispatcher = [this](const std::vector<OperationBody>& ops, const OperationOptions& options) -> void {
        std::for_each(ops.begin(), ops.end(), [this, options](const OperationBody &op) -> void {
            this->executeOperation(this->operatorRegistry->get(op.operatorNumber), const_cast<OperationBody&>(op), options);
        });
    };

    return dependenciesToReturn;
}

inline int OperatorDispatcher::getPartitionIdByKey(const SimpleString<memDbDataLength_t>& key) {
    return this->configuration->getBoolean(ConfigurationKeys::USE_PARTITIONS) ?
           this->cluster->getPartitionIdByKey(key) :
           0;
}

bool OperatorDispatcher::isAuthorizedToExecute(std::shared_ptr<Operator> operatorToExecute, AuthenticationType authenticationOfUser) {
    for(AuthenticationType authenticationTypeRequiredForOperator : operatorToExecute->desc().authorizedToExecute) {
        if(authenticationTypeRequiredForOperator == authenticationOfUser){
            return true;
        }
    }

    return false;
}

bool OperatorDispatcher::isInReplicationMode() {
    return this->configuration->getBoolean(ConfigurationKeys::USE_REPLICATION);
}

bool OperatorDispatcher::isInPartitionMode() {
    return this->configuration->getBoolean(ConfigurationKeys::USE_PARTITIONS);
}

bool OperatorDispatcher::canHoldKey(arg_t arg) {
    return this->cluster->getPartitionObject()->canHoldKey(arg);
}

bool OperatorDispatcher::canAcceptRequest() {
    return !isInReplicationMode() || NodeStates::canAcceptRequest(this->cluster->getNodeState());
}

bool OperatorDispatcher::canExecuteRequest() {
    return !isInReplicationMode() ||  NodeStates::cantExecuteRequest(this->cluster->getNodeState());
}