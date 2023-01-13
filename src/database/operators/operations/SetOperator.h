#pragma once

#include "database/operators/Operator.h"

#include <string>

class SetOperator : public Operator {
public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x01;

    Response operate(const OperationBody& operation, std::shared_ptr<Map> map) override {
        const std::string& key = std::string((char *) operation.args->at(0).arg, operation.args->at(0).lengthArg);

        int valueSize = operation.args->at(1).lengthArg;
        uint8_t * valueValue = operation.args->at(1).arg;

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