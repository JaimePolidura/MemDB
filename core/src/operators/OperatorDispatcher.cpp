#include "operators/OperatorDispatcher.h"

OperatorDispatcher::OperatorDispatcher(memDbDataStore_t dbCons, lamportClock_t clock, cluster_t cluster, configuration_t configuration,
        logger_t logger, operationLog_t operationLog): db(dbCons), operationLog(operationLog), clock(clock),
    operatorRegistry(std::make_shared<OperatorRegistry>()), logger(logger), cluster(cluster),
    configuration(configuration), delayedOperationsBuffer(std::make_shared<DelayedOperationsBuffer>())
{}

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

    bool writeDbRequestFromNode = operatorToExecute->type() == OperatorType::DB_STORE_WRITE && request.authenticationType == AuthenticationType::NODE;
    bool writeDbRequest = operatorToExecute->type() == OperatorType::DB_STORE_WRITE;
    bool readDbRequest = operatorToExecute->type() == OperatorType::DB_STORE_READ;

    OperationOptions options = {.checkTimestamps = writeDbRequestFromNode, .updateClockStrategy = LamportClock::UpdateClockStrategy::TICK};

    this->logger->debugInfo("Received request for operator {0} from {1}", request.requestNumber, operatorToExecute->name(),
                            options.checkTimestamps ? "node" : "user");

    if(isInReplicationMode() && (!canAcceptRequest() || (readDbRequest && !canExecuteRequest()))) {
        return Response::error(ErrorCode::INVALID_NODE_STATE);
    }
    if(isInReplicationMode() && isInPartitionMode() && writeDbRequest && !canHoldKey(request.operation.args->at(0))) {
        return Response::error(ErrorCode::INVALID_PARTITION);
    }
    if(isInReplicationMode() && writeDbRequest && canAcceptRequest() && !canExecuteRequest()){
        this->delayedOperationsBuffer->add(request);
        return Response::success();
    }

    return this->executeOperation(operatorToExecute, request.operation, options);
}

void OperatorDispatcher::executeOperations(std::shared_ptr<Operator> operatorToExecute,
                       const std::vector<OperationBody>& operations,
                       const OperationOptions& options) {

    for (const OperationBody& operation: operations){
        executeOperation(operatorToExecute, operation, options);
    }
}

Response OperatorDispatcher::executeOperation(std::shared_ptr<Operator> operatorToExecute,
                          const OperationBody& operation,
                          const OperationOptions& options) {

    OperatorDependencies dependencies = this->getDependencies(operatorToExecute);
    Response result = operatorToExecute->operate(operation, options, dependencies);

    this->logger->debugInfo("Executed {0} append request for operator {1} from {2}",
                            result.isSuccessful ? "successfully" : "unsuccessfully",
                            operatorToExecute->name(), options.checkTimestamps ? "node" : "user");

    if(operatorToExecute->type() == DB_STORE_WRITE && result.isSuccessful && !options.onlyExecute) {
        if(options.updateClockStrategy == LamportClock::UpdateClockStrategy::TICK){
            result.timestamp = this->clock->tick(operation.timestamp);
            operation = result.timestamp;
        } else if (options.updateClockStrategy == LamportClock::UpdateClockStrategy::SET)  {
            result.timestamp = this->clock->set(operation.timestamp);
            operation = result.timestamp;
        }

        if(!options.dontSaveInOperationLog){
            this->operationLog->add(operation);
        }

        if(isInReplicationMode() && options.fromClient() && !options.dontBroadcastToCluster){
            this->cluster->broadcast(operation);

            this->logger->debugInfo("Broadcast request for operator {0} from {1}",
                                    operatorToExecute->name(), options.checkTimestamps ? "node" : "user");
        }
    }

    return result;
}

void OperatorDispatcher::applyDelayedOperationsBuffer() {
    while(!this->delayedOperationsBuffer->isEmpty()){
        Request operation = this->delayedOperationsBuffer->get();
        this->dispatch_no_applyDelayedOperationsBuffer(operation); //Avoid recursive call
    }
}

OperatorDependencies OperatorDispatcher::getDependencies(std::shared_ptr<Operator> operatorToGetDependecies) {
    OperatorDependencies dependencies;

    for(auto dependency : operatorToGetDependecies->dependencies()){
        this->getDependency(dependency, &dependencies);
    }

    return dependencies;
}

void OperatorDispatcher::getDependency(OperatorDependency dependency, OperatorDependencies * operatorDependencies){
    switch (dependency) {
        case OPERATION_LOG:
            operatorDependencies->operationLog = this->operationLog;
            break;
        case CONFIGURATION:
            operatorDependencies->configuration = this->configuration;
            break;
        case DB_STORE:
            operatorDependencies->dbStore = this->db;
            break;
        case CLUSTER:
            operatorDependencies->cluster = this->cluster;
            break;
        case OPERATOR_DISPATCHER:
            operatorDependencies->operatorDispatcher = [this](const OperationBody& op, const OperationOptions& options) -> Response {
                return this->executeOperation(this->operatorRegistry->get(op.operatorNumber), op, options);
            };
            operatorDependencies->operatorsDispatcher = [this](const std::vector<OperationBody>& ops, const OperationOptions& options) -> void {
                std::for_each(ops.begin(), ops.end(), [this, options](const OperationBody &op) -> void {
                    this->executeOperation(this->operatorRegistry->get(op.operatorNumber), op, options);
                });
            };
            break;
    }
}

bool OperatorDispatcher::isAuthorizedToExecute(std::shared_ptr<Operator> operatorToExecute, AuthenticationType authenticationOfUser) {
    for(AuthenticationType authentationTypeRequiredForOperator : operatorToExecute->authorizedToExecute()) {
        if(authentationTypeRequiredForOperator == authenticationOfUser){
            return true;
        }
    }

    return false;
}

bool OperatorDispatcher::isInReplicationMode() {
    return this->configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_REPLICATION);
}

bool OperatorDispatcher::isInPartitionMode() {
    return this->configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_PARTITIONS);
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