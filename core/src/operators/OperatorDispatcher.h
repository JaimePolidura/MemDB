#pragma once

#include "operators/Operator.h"
#include "operators/OperatorRegistry.h"
#include "operators/DbOperatorExecutor.h"
#include "operators/MaintenanceOperatorExecutor.h"
#include "messages/response/ErrorCode.h"
#include "persistence/OperationLog.h"
#include "utils/clock/LamportClock.h"
#include "replication/Replication.h"
#include "replication/PendingReplicationOperationBuffer.h"

class OperatorDispatcher {
public: //Need it for mocking it
    operatorRegistry_t operatorRegistry;
private:
    replicationOperationBuffer_t replicationOperationBuffer;
    operationLog_t operationLog;
    configuration_t configuration;
    replication_t replication;
    lamportClock_t clock;
    memDbDataStore_t db;
    logger_t logger;

public:
    OperatorDispatcher(memDbDataStore_t dbCons, lamportClock_t clock, replication_t replication, configuration_t configuration,
                       logger_t logger, operationLog_t operationLog): db(dbCons), operationLog(operationLog), clock(clock),
                       operatorRegistry(std::make_shared<OperatorRegistry>()), logger(logger), replication(replication),
                       configuration(configuration), replicationOperationBuffer(std::make_shared<PendingReplicationOperationBuffer>())
    {}

    Response dispatch(const Request& request) {
        this->applyReplicatedOperationBuffer();

        auto operatorToExecute = this->operatorRegistry->get(request.operation.operatorNumber);

        if(operatorToExecute.get() == nullptr){
            return Response::error(ErrorCode::UNKNOWN_OPERATOR);
        }
        if(!this->isAuthorizedToExecute(operatorToExecute, request.authenticationType)) {
            return Response::error(ErrorCode::NOT_AUTHORIZED);
        }

        OperationOptions options = {.requestOfNodeToReplicate = request.authenticationType == AuthenticationType::NODE &&
                                    operatorToExecute->type() == OperatorType::WRITE};

        this->logger->debugInfo("Recieved request for operator {0} from {1}", operatorToExecute->name(),
                                options.requestOfNodeToReplicate ? "node" : "user");

        if(this->isInReplicationMode() &&
            (!NodeStates::canAcceptRequest(this->replication->getNodeState()) ||
            (!options.requestOfNodeToReplicate && !NodeStates::cantExecuteRequest(this->replication->getNodeState())))) {
            return Response::error(ErrorCode::INVALID_NODE_STATE);
        }
        if(this->isInReplicationMode() && options.requestOfNodeToReplicate &&
           !NodeStates::cantExecuteRequest(this->replication->getNodeState())){
            this->replicationOperationBuffer->add(request);
            return Response::success();
        }

        Response result = this->executeOperator(operatorToExecute, request.operation, options);

        return result;
    }

    Response executeOperator(std::shared_ptr<Operator> operatorToExecute,
                    const OperationBody& operation,
                    const OperationOptions& options) {
        Response result = this->execute(operatorToExecute, operation, options);

        this->logger->debugInfo("Executed {0} write request for operator {1} from {2}",
                                result.isSuccessful ? "successfuly" : "unsuccessfuly",
                                operatorToExecute->name(), options.requestOfNodeToReplicate ? "node" : "user");

        if(operatorToExecute->type() == WRITE && result.isSuccessful) {
            this->operationLog->add(operation);

            if(!options.requestOfNodeToReplicate) {
                result.timestamp = this->clock->tick(operation.timestamp);
            }

            if(this->isInReplicationMode() && !options.requestOfNodeToReplicate){
                this->replication->broadcast(operation);

                this->logger->debugInfo("Broadcasted request for operator {0} from {1}",
                                        operatorToExecute->name(), options.requestOfNodeToReplicate ? "node" : "user");
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

private:
    void applyReplicatedOperationBufferIfNotEmpty() {
        if(!this->replicationOperationBuffer->isEmpty()){
            this->applyReplicatedOperationBuffer();
        }
    }

    bool isAuthorizedToExecute(std::shared_ptr<Operator> operatorToExecute, AuthenticationType authenticationOfUser) {
        for(AuthenticationType authentationTypeRequiredForOperator : operatorToExecute->authorizedToExecute()) {
            if(authentationTypeRequiredForOperator == authenticationOfUser){
                return true;
            }
        }

        return false;
    }

    bool isInReplicationMode() {
        return this->configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_REPLICATION);
    }

    Response execute(std::shared_ptr<Operator> operatorToExecute, const OperationBody& operation, const OperationOptions& options) {
        return operatorToExecute->type() == OperatorType::CONTROL ?
               dynamic_cast<MaintenanceOperatorExecutor *>(operatorToExecute.get())->operate(operation, options, this->operationLog) :
               dynamic_cast<DbOperatorExecutor *>(operatorToExecute.get())->operate(operation, options, this->db);
    }
};

using operatorDispatcher_t = std::shared_ptr<OperatorDispatcher>;