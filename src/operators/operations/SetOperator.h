#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"

#include <string>

/**
 * flag1 set: key will be the hash
 */
class SetOperator : public Operator {
public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x01;

    Response operate(const OperationBody& operation, std::shared_ptr<Map> map) override {
        SimpleString key = operation.args->at(0);
        SimpleString value = operation.args->at(1);

        bool updated = map->put(key, value, operation.timestamp, operation.nodeId);

        if(updated){
            key.increaseRefCount();
            value.increaseRefCount();
        }

        return updated ?
            Response::success() :
            Response::error(ErrorCode::ALREADY_REPLICATED);
    }

    constexpr OperatorType type() override {
        return WRITE;
    }

    constexpr uint8_t operatorNumber() override {
        return OPERATOR_NUMBER;
    }
};