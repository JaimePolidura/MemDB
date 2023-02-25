#pragma once

#include "messages/request/Request.h"
#include "messages/response/Response.h"
#include "utils/datastructures/map/Map.h"
#include "operators/OperationOptions.h"
#include "auth/AuthenticationType.h"
#include "operators/buffer/OperationLogBuffer.h"

#include <memory>

enum OperatorType {
    READ, WRITE, CONTROL
};

class Operator {
public:
    //Used  for write and read operator type
    virtual Response operate(const OperationBody& operation, const OperationOptions& operationOptions, std::shared_ptr<Map> map);
    //Used for control type operators type
    virtual Response operateControl(const OperationBody& operation, const OperationOptions& operationOptions, std::shared_ptr<OperationLogBuffer> operationLogBuffer);

    virtual constexpr OperatorType type() = 0;

    virtual constexpr uint8_t operatorNumber() = 0;

    virtual AuthenticationType authorizedToExecute() = 0;
};