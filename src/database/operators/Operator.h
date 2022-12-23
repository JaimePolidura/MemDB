#pragma once

#include "messages/request/Request.h"
#include "messages/response/Response.h"
#include "../../utils/datastructures/map/Map.h"

#include <memory>

enum OperatorType {
    READ, WRITE
};

class Operator {
public:
    virtual std::shared_ptr<Response> operate(const OperationBody& operation, std::shared_ptr<Map> map) = 0;

    virtual OperatorType type() = 0;

    virtual uint8_t operatorNumber() = 0;
};