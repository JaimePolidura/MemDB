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
            printf("[SERVER] Unknown operator\n");
            onResponse(Response::error(ErrorCode::UNKNOWN_OPERATOR));
            return;
        }
        
        if(operatorToExecute->type() == READ){
            printf("[SERVER] Executing read operator\n");
            std::shared_ptr<Response> response = operatorToExecute->operate(request->operation, this->db);
            response->requestNumber = request->requestNumber;
            onResponse(response);
        }
        if(operatorToExecute->type() == WRITE){
            printf("[SERVER] Know operator write. Enqueued single thread write pool\n");
            this->singleThreadedWritePool.submit([operatorToExecute, request, onResponse, this] {
                std::shared_ptr<Response> result = operatorToExecute->operate(request->operation, this->db);
                result->requestNumber = request->requestNumber;

                onResponse(result);
            });
        }
    }
};