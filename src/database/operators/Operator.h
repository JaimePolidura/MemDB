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
    virtual Response operate(const OperationBody& operation, std::shared_ptr<Map> map) = 0;

    virtual constexpr OperatorType type() = 0;

    virtual constexpr uint8_t operatorNumber() = 0;
};