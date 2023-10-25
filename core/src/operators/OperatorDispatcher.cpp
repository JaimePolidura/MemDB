#include "operators/OperatorDispatcher.h"

OperatorDispatcher::OperatorDispatcher(memDbStores_t memDbStores, lamportClock_t clock, cluster_t cluster, configuration_t configuration,
                                       logger_t logger, operationLog_t operationLog, onGoingSyncOplogs_t onGoingSyncOplogs):
        memDbStores(memDbStores),
        operationLog(operationLog),
        onGoingSyncOplogs(onGoingSyncOplogs),
        clock(clock),
        operatorRegistry(std::make_shared<OperatorRegistry>()),
        logger(logger),
        cluster(cluster),
        configuration(configuration) {
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
            .updateClockStrategy = LamportClock::UpdateClockStrategy::TICK,
            .requestNumber = request.requestNumber,
    };

    this->logger->debugInfo("Received request for operator {0} of request number {1} from {2}", operatorToExecute->desc().name, request.requestNumber,
                            options.checkTimestamps ? "node" : "user");

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
    if(operatorToExecute->desc().type == OperatorType::DB_STORE_WRITE || operatorToExecute->desc().type == OperatorType::DB_STORE_READ){
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

    if(!options.dontDebugLog) {
        this->logger->debugInfo("Executed {0} request for operator {1} from {2}",
                                result.isSuccessful ? "successfully" : "unsuccessfully",
                                operatorToExecute->desc().name, options.checkTimestamps ? "node" : "user");
    }

    if(operatorToExecute->desc().type == DB_STORE_WRITE && result.isSuccessful && !options.onlyExecute) {
        uint64_t newTimestamp = this->updateClock(options.updateClockStrategy, operation.timestamp);
        result.timestamp = this->clock->tick(newTimestamp);
        operation.timestamp = newTimestamp;

        if(!options.dontSaveInOperationLog){
            this->operationLog->add(operation);
        }

        if(isInReplicationMode() && options.fromClient() && !options.dontBroadcastToCluster){
            this->cluster->broadcast(operation);
            this->logger->debugInfo("Broadcast request for operator {0} from {1}",
                                    operatorToExecute->desc().name, options.checkTimestamps ? "node" : "user");
        }
    }

    return result;
}

uint64_t OperatorDispatcher::updateClock(LamportClock::UpdateClockStrategy strategy, uint64_t newValue) {
    if(strategy == LamportClock::UpdateClockStrategy::TICK){
        return this->clock->tick(newValue);
    } else if (strategy == LamportClock::UpdateClockStrategy::SET) {
        return this->clock->set(newValue);
    }

    return this->clock->getCounterValue();
}

void OperatorDispatcher::applyDelayedOperationsBuffer() {
    while(!this->delayedOperationsBuffer.isEmpty()){
        Request operation = this->delayedOperationsBuffer.get();
        this->dispatch_no_applyDelayedOperationsBuffer(operation); //Avoid recursive call
    }
}

OperatorDependencies OperatorDispatcher::getDependencies() {
    OperatorDependencies dependenciesToReturn{};

    dependenciesToReturn.onGoingSyncOplogs = this->onGoingSyncOplogs;
    dependenciesToReturn.configuration = this->configuration;
    dependenciesToReturn.operationLog = this->operationLog;
    dependenciesToReturn.cluster = this->cluster;
    dependenciesToReturn.logger = this->logger;
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