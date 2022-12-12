#pragma once

#include "Operator.h"
#include "OperatorRegistry.h"

#include <string>

class GetOperator : public Operator {
public:
    static const uint8_t OPERATOR_NUMBER = 0x02;

    GetOperator() {
        STORE_OPERATOR(OPERATOR_NUMBER, std::shared_ptr<GetOperator>(this), operators);
    }

    std::shared_ptr<Response> operate(const OperationBody& operation, Map& map) override {

    }

    OperatorType type() override {
        return READ;
    }

    uint8_t operatorNumber() override {
        return OPERATOR_NUMBER;
    }
};