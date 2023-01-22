#pragma once

#include "operators/Operator.h"

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

        key.increaseRefCount();
        value.increaseRefCount();

        map->put(key, value);

        return Response::success();
    }

    constexpr OperatorType type() override {
        return WRITE;
    }

    constexpr uint8_t operatorNumber() override {
        return OPERATOR_NUMBER;
    }
};