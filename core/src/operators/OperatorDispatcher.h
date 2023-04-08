#pragma once

#include "operators/Operator.h"
#include "operators/OperatorRegistry.h"
#include "operators/DbOperator.h"
#include "operators/ControlOperator.h"
#include "messages/response/ErrorCode.h"
#include "operators/buffer/OperationLogBuffer.h"
#include "utils/clock/LamportClock.h"
#include "replication/Replication.h"
#include "replication/PendingReplicationOperationBuffer.h"

class OperatorDispatcher {
private:
    replicationOperationBuffer_t replicationOperationBuffer;
    operationLogBuffer_t operationLogBuffer;
    operatorRegistry_t operatorRegistry;
    configuration_t configuration;
    replication_t replication;
    lamportClock_t clock;
    memDbDataStore_t db;
    logger_t logger;

public:
    OperatorDispatcher(memDbDataStore_t dbCons, lamportClock_t clock, operationLogBuffer_t operationLogBuffer,
                       replication_t replication, configuration_t configuration, logger_t logger):
            db(dbCons), operationLogBuffer(operationLogBuffer), clock(clock), operatorRegistry(std::make_shared<OperatorRegistry>()), logger(logger),
            replication(replication), configuration(configuration), replicationOperationBuffer(std::make_shared<PendingReplicationOperationBuffer>())
    {}

    OperatorDispatcher(memDbDataStore_t dbCons, lamportClock_t clock, operationLogBuffer_t operationLogBuffer,
                       operatorRegistry_t operatorRegistry, replication_t replication, configuration_t configuration, logger_t logger):
            db(dbCons), operationLogBuffer(operationLogBuffer), clock(clock), operatorRegistry(operatorRegistry), logger(logger),
            replication(replication), configuration(configuration), replicationOperationBuffer(std::make_shared<PendingReplicationOperationBuffer>())
    {}

    Response dispatch(const Request& request) {
        this->applyReplicatedOperationBuffer();

        auto operatorToExecute = this->operatorRegistry->get(request.operation.operatorNumber);

        if(operatorToExecute.get() == nullptr){
            return Response::error(ErrorCode::UNKNOWN_OPERATOR);
        }
        if(operatorToExecute.get()->authorizedToExecute() != request.authenticationType) {
            return Response::error(ErrorCode::NOT_AUTHORIZED);
        }

        OperationOptions options = {.requestFromReplication = request.authenticationType == AuthenticationType::CLUSTER};

        if(this->isInReplicationMode() &&
            (!NodeStates::canAcceptRequest(this->replication->getNodeState()) ||
            (!options.requestFromReplication && !NodeStates::cantExecuteRequest(this->replication->getNodeState())))) {
            return Response::error(ErrorCode::INVALID_NODE_STATE);
        }
        if(this->isInReplicationMode() && options.requestFromReplication &&
            !NodeStates::cantExecuteRequest(this->replication->getNodeState())){
            this->replicationOperationBuffer->add(request);
            return Response::success();
        }

        Response result = this->execute(operatorToExecute, request.operation, options);

        if(operatorToExecute->type() == WRITE && result.isSuccessful) {
            this->operationLogBuffer->add(request.operation);

            if(!options.requestFromReplication) {
                result.timestamp = this->clock->tick(request.operation.timestamp);
            }
            if(this->isInReplicationMode() && !options.requestFromReplication){
                this->replication->broadcast(request);
            }
        }

        return result;
    }

    void applyReplicatedOperationBuffer() {
        while(!this->replicationOperationBuffer->isEmpty()){
            Request operation = this->replicationOperationBuffer->get();
            this->dispatch(operation);
        }
    }

    Response executeOperator(const OperationOptions &options, const OperationBody &operationBody) {
        std::shared_ptr<Operator> operatorToExecute = this->operatorRegistry->get(operationBody.operatorNumber);

        return this->execute(operatorToExecute, operationBody, options);
    }
private:
    void applyReplicatedOperationBufferIfNotEmpty() {
        if(!this->replicationOperationBuffer->isEmpty()){
            this->applyReplicatedOperationBuffer();
        }
    }

    bool isInReplicationMode() {
        return this->configuration->getBoolean(ConfigurationKeys::USE_REPLICATION);
    }

    Response execute(std::shared_ptr<Operator> operatorToExecute, const OperationBody& operation, const OperationOptions& options) {
        return operatorToExecute->type() == OperatorType::CONTROL ?
               dynamic_cast<ControlOperator *>(operatorToExecute.get())->operate(operation, options, this->operationLogBuffer) :
               dynamic_cast<DbOperator *>(operatorToExecute.get())->operate(operation, options, this->db);
    }
};

using operatorDispatcher_t = std::shared_ptr<OperatorDispatcher>;