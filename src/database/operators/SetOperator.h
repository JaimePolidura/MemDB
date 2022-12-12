#pragma once

#include "Operator.h"
#include "OperatorRegistry.h"

#include <string>


class SetOperator : public Operator {
public:
    static const uint8_t OPERATOR_NUMBER = 0x01;

    SetOperator() {
        STORE_OPERATOR(OPERATOR_NUMBER, std::shared_ptr<SetOperator>(this), operators);
    }

    std::shared_ptr<Response> operate(const OperationBody& operation, Map& map) override {
        const std::string& key = std::string((char *) operation.args[0].arg, operation.args[0].lengthArg);

        int valueSize = operation.args[1].lengthArg;
        uint8_t * valueValue = operation.args[1].arg;

        map.put(key, valueValue, valueSize);

        return std::make_shared<Response>();
    }

    OperatorType type() override {
        return WRITE;
    }

    uint8_t operatorNumber() override {
        return OPERATOR_NUMBER;
    }
};