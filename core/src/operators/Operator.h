#pragma once

#include "operators/OperationOptions.h"
#include "auth/AuthenticationType.h"
#include "shared.h"

enum OperatorType {
    READ, WRITE, CONTROL
};

class Operator {
public:
    virtual OperatorType type() = 0;

    virtual uint8_t operatorNumber() = 0;

    virtual std::vector<AuthenticationType> authorizedToExecute() = 0;

    virtual std::string name() = 0;
};