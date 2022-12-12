#pragma once

#include "Operator.h"
#include "OperatorRegistry.h"

#include <string>

class GetOperator : public Operator {
    void operate(const OperationBody& operation, Map& map) override {

    }

    OperatorType type() override {
        return READ;
    }

    uint8_t operatorNumber() override {
        return 0x02;
    }
};