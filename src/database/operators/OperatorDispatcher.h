#pragma once

#include <memory>
#include <vector>
#include <map>
#include <functional>

#include "Operator.h"
#include "../../utils/threads/dynamicthreadpool/SingleThreadPool.h"
#include "../../messages/response/ErrorCode.h"
#include "OperatorRegistry.h"

class OperatorDispatcher {
private:
    std::shared_ptr<Map> db;
    std::shared_ptr<OperatorRegistry> operatorRegistry;
    SingleThreadPool singleThreadedWritePool;

public:
    OperatorDispatcher(std::shared_ptr<Map> dbCons, std::shared_ptr<OperatorRegistry> operatorRegistryCons):
        db(dbCons),
        operatorRegistry(operatorRegistryCons) {}

    void dispatch(Request& request,
                  std::function<void(std::shared_ptr<Response>)> onResponse) {
        std::shared_ptr<Operator> operatorToExecute = this->operatorRegistry->get(request.operation.operatorNumber);
        uint64_t requestNumber = request.requestNumber;

        if(operatorToExecute.get() == nullptr){
            std::shared_ptr<Response> result = Response::error(ErrorCode::UNKNOWN_OPERATOR);
            this->callOnResponseCallback(onResponse, result, requestNumber);
            return;
        }

        if(operatorToExecute->type() == READ){
            std::shared_ptr<Response> result = operatorToExecute->operate(request.operation, this->db);
            this->callOnResponseCallback(onResponse, result, requestNumber);
        }
        if(operatorToExecute->type() == WRITE){
            this->singleThreadedWritePool.submit([operatorToExecute, onResponse, requestNumber, request, this] {
                std::shared_ptr<Response> result = operatorToExecute->operate(request.operation, this->db);
                this->callOnResponseCallback(onResponse, result, requestNumber);
            });
        }
    }

private:
    class Runner {
        std::shared_ptr<Operator> operatorToExecute;
        std::function<void(std::shared_ptr<Response>)> onResponse;
        uint64_t requestNumber;
    };

    void callOnResponseCallback(std::function<void(std::shared_ptr<Response>)> onResponse,
                                std::shared_ptr<Response> result,
                                uint64_t requestNumber) {
        result->requestNumber = requestNumber;
        onResponse(result);
    }
};