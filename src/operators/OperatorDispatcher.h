#pragma once

#include <memory>
#include <vector>
#include <map>
#include <functional>

#include "Operator.h"
#include "OperatorRegistry.h"
#include "utils/threads/dynamicthreadpool/SingleThreadPool.h"
#include "messages/response/ErrorCode.h"
#include "persistence/OperationLogSaver.h"
#include "utils/clock/LamportClock.h"

class OperatorDispatcher {
private:
    std::shared_ptr<LamportClock> clock;
    std::shared_ptr<Map> db;
    OperatorRegistry operatorRegistry;
    OperationLogSaver operationLogSaver;

public:
    OperatorDispatcher(std::shared_ptr<Map> dbCons, std::shared_ptr<LamportClock> clock, const OperationLogSaver& operationLogSaver):
        db(dbCons), operationLogSaver(std::move(operationLogSaver)), clock(clock)
    {}

    void dispatch(Request& request,
                  std::function<void(Response&)> onResponse) {
        std::shared_ptr<Operator> operatorToExecute = this->operatorRegistry.get(request.operation.operatorNumber);
        uint64_t requestNumber = request.requestNumber;

        if(operatorToExecute.get() == nullptr){
            this->decreaseRequestArgumentsRefCount(request);
            Response result = Response::error(ErrorCode::UNKNOWN_OPERATOR);
            this->callOnResponseCallback(onResponse, result, requestNumber);
            return;
        }

        Response result = operatorToExecute->operate(request.operation, this->db);

        if(operatorToExecute->type() == WRITE && result.isSuccessful) {
            this->operationLogSaver.save(request);
        }

        this->callOnResponseCallback(onResponse, result, requestNumber);
        
        this->decreaseRequestArgumentsRefCount(request);
        result.responseValue.decreaseRefCount();
    }

    Response executeOperator(std::shared_ptr<Map> map, const OperationBody& operationBody) {
        std::shared_ptr<Operator> operatorToExecute = this->operatorRegistry.get(operationBody.operatorNumber);

        return operatorToExecute->operate(operationBody, map);
    }

private:
    void decreaseRequestArgumentsRefCount(Request &request) {
        std::shared_ptr<std::vector<SimpleString>> args = request.operation.args;

        for(int i = 0; i < args->size(); i++)
            args->at(i).decreaseRefCount();
    }

    void callOnResponseCallback(const std::function<void(Response&)>& onResponse,
                                Response& result,
                                uint64_t requestNumber) {
        result.requestNumber = requestNumber;
        onResponse(result);
    }
};