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

    void dispatch(std::shared_ptr<Request> request,
                  std::function<void(std::shared_ptr<Response>)> onResponse) {
        std::shared_ptr<Operator> operatorToExecute = this->operatorRegistry->get(request->operation->operatorNumber);

        if(operatorToExecute.get() == nullptr){
            std::shared_ptr<Response> result = Response::error(ErrorCode::UNKNOWN_OPERATOR);
            this->callOnResponseCallback(onResponse, result, request);
            return;
        }

        if(operatorToExecute->type() == READ){
            std::shared_ptr<Response> result = operatorToExecute->operate(request->operation, this->db);
            this->callOnResponseCallback(onResponse, result, request);
        }
        if(operatorToExecute->type() == WRITE){
            this->singleThreadedWritePool.submit([operatorToExecute, request, onResponse, this] {
                std::shared_ptr<Response> result = operatorToExecute->operate(request->operation, this->db);
                this->callOnResponseCallback(onResponse, result, request);
            });
        }
    }

private:
    void callOnResponseCallback(std::function<void(std::shared_ptr<Response>)> onResponse,
                                std::shared_ptr<Response> result,
                                std::shared_ptr<Request> request) {
        result->requestNumber = request->requestNumber;
        onResponse(result);
    }
};