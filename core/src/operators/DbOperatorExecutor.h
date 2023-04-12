#pragma once

#include "messages/request/Request.h"
#include "messages/response/Response.h"
#include "utils/datastructures/map/Map.h"
#include "operators/OperationOptions.h"

class DbOperatorExecutor {
public:
    virtual Response operate(const OperationBody& operation, const OperationOptions& operationOptions, memDbDataStore_t map) = 0;
};