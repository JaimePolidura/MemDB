#pragma once

#include "messages/request/Request.h"
#include "messages/response/Response.h"
#include "../../utils/datastructures/map/Map.h"

#include <memory>

enum OperatorType {
    READ, WRITE
};

#define STORE_OPERATOR(operatorNumber, operatorInstance, operatorsMap) operatorsMap[operatorNumber]=operatorInstance

class Operator {
    virtual std::shared_ptr<Response> operate(const OperationBody& operation, Map& map) = 0;

    virtual OperatorType type() = 0;

    virtual uint8_t operatorNumber() = 0;
};