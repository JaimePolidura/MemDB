#pragma once

#include <memory>
#include <vector>
#include <map>
#include <functional>

#include "Operator.h"
#include "connection/TCPConnection.h"
#include "../../utils/threads/dynamicthreadpool/SingleThreadPool.h"

static std::map<int, std::shared_ptr<Operator>> operators;

class OperatorDispatcher {
private:
    std::shared_ptr<Map> db;
    SingleThreadPool singleThreadedWritePool;

public:
    OperatorDispatcher(std::shared_ptr<Map> dbCons): db(dbCons) {}

    void dispatch(std::shared_ptr<Request> request,
                  const std::shared_ptr<TCPConnection>& connection,
                  std::function<void(std::shared_ptr<Response>)> onResponse) {

        std::shared_ptr<Operator> operatorToExecute = operators[request->operation[0].operatorNumber];
        if(operatorToExecute.get() == nullptr){
            //TODO error
            return;
        }

        if(operatorToExecute->type() == READ){
            onResponse(operatorToExecute->operate(* request->operation, this->db));
        }
        if(operatorToExecute->type() == WRITE){
            this->singleThreadedWritePool.submit([&] {
                onResponse(operatorToExecute->operate(* request->operation, this->db));
            });
        }


    }
};