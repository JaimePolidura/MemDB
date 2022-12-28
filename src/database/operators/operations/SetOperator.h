#pragma once

#include "database/operators/Operator.h"

#include <string>

class SetOperator : public Operator {
public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x01;

    std::shared_ptr<Response> operate(const OperationBody& operation, std::shared_ptr<Map> map) override {
        const std::string& key = std::string((char *) operation.args[0].arg, operation.args[0].lengthArg);

        int valueSize = operation.args[1].lengthArg;
        uint8_t * valueValue = operation.args[1].arg;

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