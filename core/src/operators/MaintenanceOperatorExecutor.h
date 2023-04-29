#pragma once

#include "messages/request/Request.h"
#include "messages/response/Response.h"
#include "operators/OperationOptions.h"
#include "persistence/oplog/SingleOperationLog.h"

class MaintenanceOperatorExecutor {
public:
    virtual Response operate(const OperationBody& operation, const OperationOptions& operationOptions, operationLog_t operationLog) = 0;
};