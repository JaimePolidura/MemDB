#pragma once

#include "operators/OperationOptions.h"
#include "auth/AuthenticationType.h"
#include "messages/response/Response.h"
#include "messages/multi/MultipleResponseSenderIterator.h"
#include "utils/datastructures/map/Map.h"
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
    bool isMultiResponsesFragment;
};

class Operator {
public:
    virtual Response operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies& dependencies) = 0;

    virtual multipleResponseSenderIterator_t multiResponseSenderIterator(const OperationBody& operation, OperatorDependencies& dependencies);

    virtual constexpr OperatorDescriptor desc() = 0;
};

using operator_t = std::shared_ptr<Operator>;