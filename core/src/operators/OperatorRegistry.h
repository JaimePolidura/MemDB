#pragma once

#include "shared.h"

#include "operators/Operator.h"
#include "operators/AllOperators.h"

class OperatorRegistry {
private:
    std::map<uint8_t, std::shared_ptr<Operator>> operators;

public:
    OperatorRegistry();

    virtual ~OperatorRegistry() = default;

    virtual std::shared_ptr<Operator> get(uint8_t operatorNumber);
};

using operatorRegistry_t = std::shared_ptr<OperatorRegistry>;