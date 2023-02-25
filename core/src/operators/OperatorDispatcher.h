#pragma once

#include <memory>
#include <vector>
#include <map>
#include <functional>

#include "Operator.h"
#include "OperatorRegistry.h"
#include "utils/threads/dynamicthreadpool/SingleThreadPool.h"
#include "messages/response/ErrorCode.h"
#include "operators/buffer/OperationLogBuffer.h"
#include "utils/clock/LamportClock.h"

class OperatorDispatcher {
private:
    std::shared_ptr<OperationLogBuffer> operationLogBuffer;
    std::shared_ptr<LamportClock> clock;
    OperatorRegistry operatorRegistry;
    std::shared_ptr<Map> db;

public:
    OperatorDispatcher(std::shared_ptr<Map> dbCons, std::shared_ptr<LamportClock> clock, std::shared_ptr<OperationLogBuffer> operationLogBuffer):
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

        //TODO Improve
        Response result = operatorToExecute->type() == OperatorType::CONTROL ?
                operatorToExecute->operateControl(request.operation, options, this->operationLogBuffer) :
                operatorToExecute->operate(request.operation, options, this->db);

        if(operatorToExecute->type() == WRITE && result.isSuccessful) {
            this->operationLogBuffer->add(request.operation);

            if(!options.requestFromReplication) {
                uint64_t actualCount = this->clock->tick(request.operation.timestamp);
                result.timestamp = actualCount;
            }
        }

        this->callOnResponseCallback(onResponse, result, requestNumber);
    }

    Response executeOperator(std::shared_ptr<Map> map, const OperationOptions& options, const OperationBody& operationBody) {
        std::shared_ptr<Operator> operatorToExecute = this->operatorRegistry.get(operationBody.operatorNumber);

        return operatorToExecute->operate(operationBody, options, map);
    }

private:
    void callOnResponseCallback(const std::function<void(Response&)>& onResponse,
                                Response& result,
                                uint64_t requestNumber) {
        result.requestNumber = requestNumber;
        onResponse(result);
    }
};