#pragma once

#include <vector>
#include <memory>

#include "Operator.h"
#include "database/operators/operations/GetOperator.h"
#include "database/operators/operations/SetOperator.h"
#include "database/operators/operations/DeleteOperator.h"

class AllOperators {
public:
    static std::vector<std::shared_ptr<Operator>> list() {
        return {
            std::make_shared<GetOperator>(),
            std::make_shared<SetOperator>(),
            std::make_shared<DeleteOperator>()
        };
    }
};