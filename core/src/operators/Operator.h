#pragma once

#include "operators/OperationOptions.h"
#include "auth/AuthenticationType.h"

enum OperatorType {
    READ, WRITE, CONTROL
};

class Operator {
public:
    virtual constexpr OperatorType type() = 0;

    virtual constexpr uint8_t operatorNumber() = 0;

    virtual AuthenticationType authorizedToExecute() = 0;
};