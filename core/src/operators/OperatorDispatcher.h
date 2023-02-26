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
    OperatorRegistry operatorRegistry;
    memDbDataStore_t db;
    lamportClock_t clock;

public:
    OperatorDispatcher(memDbDataStore_t dbCons, lamportClock_t clock, operationLogBuffer_t operationLogBuffer):
        db(dbCons), operationLogBuffer(operationLogBuffer), clock(clock)
    {}

    void dispatch(Request& request,
                  std::function<void(Response&)> onResponse) {
        std::shared_ptr<Operator> operatorToExecute = this->operatorRegistry.get(request.operation.operatorNumber);
        uint64_t requestNumber = request.requestNumber;

        if(operatorToExecute.get() == nullptr){ //Check if operator exists
            Response result = Response::error(ErrorCode::UNKNOWN_OPERATOR);
            this->callOnResponseCallback(onResponse, result, requestNumber);
            return;
        }
        if(operatorToExecute.get()->authorizedToExecute() != request.authenticationType) { //Check if it has permissions
            Response result = Response::error(ErrorCode::NOT_AUTHORIZED);
            this->callOnResponseCallback(onResponse, result, requestNumber);
            return;
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

        this->callOnResponseCallback(onResponse, result, requestNumber);
    }

    Response executeOperator(memDbDataStore_t map, const OperationOptions& options, const OperationBody& operationBody) {
        std::shared_ptr<Operator> operatorToExecute = this->operatorRegistry.get(operationBody.operatorNumber);

        return this->execute(operatorToExecute, operationBody, options);
    }

private:
    Response execute(std::shared_ptr<Operator> operatorToExecute, const OperationBody& operation, const OperationOptions& options) {
        return operatorToExecute->type() == OperatorType::CONTROL ?
               dynamic_cast<ControlOperator *>(operatorToExecute.get())->operate(operation, options, this->operationLogBuffer) :
               dynamic_cast<DbOperator *>(operatorToExecute.get())->operate(operation, options, this->db);
    }

    void callOnResponseCallback(const std::function<void(Response&)>& onResponse,
                                Response& result,
                                uint64_t requestNumber) {
        result.requestNumber = requestNumber;
        onResponse(result);
    }
};

using operatorDispatcher_t = std::shared_ptr<OperatorDispatcher>;