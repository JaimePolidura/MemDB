#pragma once

#include <vector>
#include <memory>

#include "Operator.h"
#include "GetOperator.h"
#include "SetOperator.h"

class AllOperators {
public:
    static std::vector<std::shared_ptr<Operator>> list() {
        return {
            std::make_shared<GetOperator>(),
            std::make_shared<SetOperator>()
        };
    }
};