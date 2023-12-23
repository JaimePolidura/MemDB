#include "operators/OperatorDispatcher.h"

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
            .partitionId = 0,
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
    if(!options.fromClient() && isInReplicationMode()) {
        this->cluster->checkHintedHandoff(request.operation.nodeId);
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

    bool canBroadcast = operatorToExecute->hasProperty(OperatorProperty::BROADCAST) && isInReplicationMode() &&
        result.isSuccessful && !options.dontBroadcastToCluster;
    bool canBePersisted = operatorToExecute->hasProperty(OperatorProperty::PERSISTENCE) && result.isSuccessful &&
        !options.dontSaveInOperationLog;

    if(options.fromClient() && (canBroadcast || canBePersisted)) {
        operation.timestamp = result.timestamp.counter;
    }
    if(canBePersisted) {
        this->operationLog->add(options.partitionId, operation);
    }
    if(canBroadcast) {
        multipleResponses_t responses = this->cluster->broadcastAndWait(operation, {
                        .partitionId = this->getPartitionIdByKey(operation.getArg(0)),
                        .canBeStoredInHint =  true});
        this->logger->debugInfo("Broadcasting request for operator {0} of key {1} with timestamp ({2}, {3})",
                                operatorToExecute->desc().name, operation.getArg(0).toString(), operation.timestamp, cluster->getNodeId());
    }

    return result;
}

void OperatorDispatcher::applyDelayedOperationsBuffer() {
    iterator_t<Request> iterator = this->delayedOperationsBuffer.iterator();

    while(iterator->hasNext()) {
        Request operation = iterator->next();
        this->dispatch_no_applyDelayedOperationsBuffer(operation);
    }
}

void OperatorDispatcher::buildDependencies() {
    OperatorDependencies dependencies{};

    dependencies.onGoingPaxosRounds = this->onGoingPaxosRounds;
    dependencies.onGoingSyncOplogs = this->onGoingSyncOplogs;
    dependencies.configuration = this->configuration;
    dependencies.operationLog = this->operationLog;
    dependencies.cluster = this->cluster;
    dependencies.logger = this->logger;
    dependencies.clock = this->clock;
    dependencies.memDbStores = this->memDbStores;
    dependencies.operatorDispatcher = [this](const OperationBody& op, const OperationOptions& options) -> Response {
        return this->executeOperation(this->operatorRegistry->get(op.operatorNumber), const_cast<OperationBody&>(op), options);
    };
    dependencies.operatorsDispatcher = [this](const std::vector<OperationBody>& ops, const OperationOptions& options) -> void {
        std::for_each(ops.begin(), ops.end(), [this, options](const OperationBody &op) -> void {
            this->executeOperation(this->operatorRegistry->get(op.operatorNumber), const_cast<OperationBody&>(op), options);
        });
    };

    this->dependencies = dependencies;
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