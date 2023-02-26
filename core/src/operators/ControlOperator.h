#pragma once

#include "messages/request/Request.h"
#include "messages/response/Response.h"
#include "operators/OperationOptions.h"
#include "operators/buffer/OperationLogBuffer.h"

class ControlOperator {
public:
    virtual Response operate(const OperationBody& operation, const OperationOptions& operationOptions, operationLogBuffer_t operationLogBuffer) = 0;
};