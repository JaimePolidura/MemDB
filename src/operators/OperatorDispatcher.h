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

class OperatorDispatcher {
private:
    std::shared_ptr<Map> db;
    OperatorRegistry operatorRegistry;
    OperationLogSaver operationLogSaver;

public:
    OperatorDispatcher(std::shared_ptr<Map> dbCons, const OperationLogSaver& operationLogSaver):
        db(dbCons), operationLogSaver(std::move(operationLogSaver))
    {}

    void dispatch(Request& request,
                  std::function<void(Response&)> onResponse) {
        std::shared_ptr<Operator> operatorToExecute = this->operatorRegistry.get(request.operation.operatorNumber);
        uint64_t requestNumber = request.requestNumber;

        if(operatorToExecute.get() == nullptr){
            Response result = Response::error(ErrorCode::UNKNOWN_OPERATOR);
            this->callOnResponseCallback(onResponse, result, requestNumber);
            return;
        }

        if(operatorToExecute->type() == WRITE)
            this->operationLogSaver.save(request);

        Response result = operatorToExecute->operate(request.operation, this->db);
        this->callOnResponseCallback(onResponse, result, requestNumber);

        this->decreaseRequestArgumentsRefCount(request);
    }

    Response executeOperator(std::shared_ptr<Map> map, const OperationBody& operationBody) {
        std::shared_ptr<Operator> operatorToExecute = this->operatorRegistry.get(operationBody.operatorNumber);

        return operatorToExecute->operate(operationBody, map);
    }

private:
    void decreaseRequestArgumentsRefCount(Request &request) {
        std::shared_ptr<std::vector<SmallString>> args = request.operation.args;

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