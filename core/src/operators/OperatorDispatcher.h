#pragma once

#include "Operator.h"
#include "OperatorRegistry.h"
#include "DbOperator.h"
#include "ControlOperator.h"
#include "messages/response/ErrorCode.h"
#include "operators/buffer/OperationLogBuffer.h"
#include "utils/clock/LamportClock.h"

class OperatorDispatcher {
private:
    operationLogBuffer_t operationLogBuffer;
    operatorRegistry_t operatorRegistry;
    memDbDataStore_t db;
    lamportClock_t clock;

public:
    OperatorDispatcher(memDbDataStore_t dbCons, lamportClock_t clock, operationLogBuffer_t operationLogBuffer):
        db(dbCons), operationLogBuffer(operationLogBuffer), clock(clock), operatorRegistry(std::make_shared<OperatorRegistry>())
    {}

    OperatorDispatcher(memDbDataStore_t dbCons, lamportClock_t clock, operationLogBuffer_t operationLogBuffer, operatorRegistry_t operatorRegistry):
            db(dbCons), operationLogBuffer(operationLogBuffer), clock(clock), operatorRegistry(operatorRegistry)
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
                uint64_t actualCount = this->clock->tick(request.operation.timestamp);
                result.timestamp = actualCount;
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