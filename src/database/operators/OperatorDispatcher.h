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
    SingleThreadPool singleThreadedWritePool;
    OperationLogSaver operationLogSaver;

public:
    OperatorDispatcher(std::shared_ptr<Map> dbCons, const OperationLogSaver& operationLogSaver):
        db(dbCons), singleThreadedWritePool("OperatorDispatcher"),operationLogSaver(std::move(operationLogSaver))
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

        if(operatorToExecute->type() == READ){
            Response result = operatorToExecute->operate(request.operation, this->db);
            this->callOnResponseCallback(onResponse, result, requestNumber);
        }
        if(operatorToExecute->type() == WRITE){
            this->singleThreadedWritePool.submit([operatorToExecute, onResponse, requestNumber, request, this] {
                this->operationLogSaver.save(request);
                
                Response result = operatorToExecute->operate(request.operation, this->db);

                this->callOnResponseCallback(onResponse, result, requestNumber);
            });
        }
    }

private:
    void callOnResponseCallback(const std::function<void(Response&)>& onResponse,
                                Response& result,
                                uint64_t requestNumber) {
        result.requestNumber = requestNumber;
        onResponse(result);
    }
};