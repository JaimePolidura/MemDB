#pragma once

#include "operators/OperationOptions.h"
#include "operators/dependencies/OperatorDependency.h"
#include "operators/dependencies/OperatorDependencies.h"
#include "auth/AuthenticationType.h"
#include "messages/response/Response.h"
#include "utils/datastructures/map/Map.h"
#include "shared.h"
#include "persistence/oplog/OperationLog.h"

enum OperatorType {
    READ, WRITE, CONTROL
};

class Operator {
public:
    virtual Response operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies dependencies) = 0;

    virtual OperatorType type() = 0;

    virtual uint8_t operatorNumber() = 0;

    virtual std::vector<AuthenticationType> authorizedToExecute() = 0;

    virtual std::vector<OperatorDependency> dependencies() = 0;

    virtual std::string name() = 0;
};