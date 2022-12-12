#pragma once

#include "messages/request/Request.h"
#include "../../utils/datastructures/map/Map.h"

enum OperatorType {
    READ, WRITE
};

#define STORE_OPERATOR(operatorNumber, operatorInstance, operatorsMap) operatorsMap[operatorNumber]=operatorInstance

class Operator {
    virtual void operate(const OperationBody& operation, Map& map) = 0;

    virtual OperatorType type() = 0;

    virtual uint8_t operatorNumber() = 0;
};