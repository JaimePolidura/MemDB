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
        int valueSize = operation.args->at(1).size;
        uint8_t * valueValue = (uint8_t *) operation.args->at(1).value;

        SmallString key = operation.args->at(0);
        key.timesToBeDeleted = 2;

        map->put(key, valueValue, valueSize);

        return Response::success();
    }

    constexpr OperatorType type() override {
        return WRITE;
    }

    constexpr uint8_t operatorNumber() override {
        return OPERATOR_NUMBER;
    }
};