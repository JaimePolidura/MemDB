#pragma once

#include "Operator.h"
#include "OperatorRegistry.h"
#include "DbOperator.h"
#include "ControlOperator.h"
#include "messages/response/ErrorCode.h"
#include "operators/buffer/OperationLogBuffer.h"
#include "utils/clock/LamportClock.h"
#include "replication/Replication.h"

class OperatorDispatcher {
private:
    operationLogBuffer_t operationLogBuffer;
    operatorRegistry_t operatorRegistry;
    configuration_t configuration;
    replication_t replication;
    lamportClock_t clock;
    memDbDataStore_t db;

public:
    OperatorDispatcher(memDbDataStore_t dbCons, lamportClock_t clock, operationLogBuffer_t operationLogBuffer,
                       replication_t replication, configuration_t configuration):
        db(dbCons), operationLogBuffer(operationLogBuffer), clock(clock), operatorRegistry(std::make_shared<OperatorRegistry>()),
        replication(replication), configuration(configuration)
    {}

    OperatorDispatcher(memDbDataStore_t dbCons, lamportClock_t clock, operationLogBuffer_t operationLogBuffer,
                       operatorRegistry_t operatorRegistry, replication_t replication, configuration_t configuration):
            db(dbCons), operationLogBuffer(operationLogBuffer), clock(clock), operatorRegistry(operatorRegistry),
            replication(replication), configuration(configuration)
    {}

    Response dispatch(const Request& request) {
        auto operatorToExecute = this->operatorRegistry->get(request.operation.operatorNumber);

        if(operatorToExecute.get() == nullptr){
            return Response::error(ErrorCode::UNKNOWN_OPERATOR);
        }
        if(operatorToExecute.get()->authorizedToExecute() != request.authenticationType) {
            return Response::error(ErrorCode::NOT_AUTHORIZED);
        }

        OperationOptions options = {
                .requestFromReplication = request.authenticationType == AuthenticationType::CLUSTER
        };

        Response result = this->execute(operatorToExecute, request.operation, options);

        if(operatorToExecute->type() == WRITE && result.isSuccessful) {
            this->operationLogBuffer->add(request.operation);

            if(!options.requestFromReplication) {
                result.timestamp = this->clock->tick(request.operation.timestamp);
            }
            if(this->configuration->getBoolean(ConfigurationKeys::USE_REPLICATION) && !options.requestFromReplication){
                this->replication->broadcast(request);
            }
        }

        return result;
    }

    Response executeOperator(memDbDataStore_t map, const OperationOptions& options, const OperationBody& operationBody) {
        std::shared_ptr<Operator> operatorToExecute = this->operatorRegistry->get(operationBody.operatorNumber);

        return this->execute(operatorToExecute, operationBody, options);
    }
private:
    Response execute(std::shared_ptr<Operator> operatorToExecute, const OperationBody& operation, const OperationOptions& options) {
        return operatorToExecute->type() == OperatorType::CONTROL ?
               dynamic_cast<ControlOperator *>(operatorToExecute.get())->operate(operation, options, this->operationLogBuffer) :
               dynamic_cast<DbOperator *>(operatorToExecute.get())->operate(operation, options, this->db);
    }
};

using operatorDispatcher_t = std::shared_ptr<OperatorDispatcher>;