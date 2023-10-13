#pragma once

#include "operators/OperationOptions.h"
#include "auth/AuthenticationType.h"
#include "messages/response/Response.h"
#include "utils/Iterator.h"
#include "shared.h"
#include "persistence/OperationLog.h"
#include "OperatorDependencies.h"

enum OperatorType {
    DB_STORE_READ, DB_STORE_WRITE, NODE_MAINTENANCE
};

struct OperatorDescriptor {
    OperatorType type;
    uint8_t number;
    std::string name;
    std::vector<AuthenticationType> authorizedToExecute;
};

class Operator {
public:
    virtual Response operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies& dependencies) = 0;

    virtual iterator_t<std::vector<uint8_t>> createMultiResponseSenderIterator(const OperationBody& operation, OperatorDependencies& dependencies);

    virtual OperatorDescriptor desc() = 0;
};

using operator_t = std::shared_ptr<Operator>;