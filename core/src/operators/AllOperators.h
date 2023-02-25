#pragma once

#include <vector>
#include <memory>

#include "Operator.h"
#include "operators/operations/GetOperator.h"
#include "operators/operations/SetOperator.h"
#include "operators/operations/DeleteOperator.h"
#include "operators/operations/HealthCheckOperator.h"
#include "operators/operations/SyncDataOperator.h"

class AllOperators {
public:
    static std::vector<std::shared_ptr<Operator>> list() {
        return {
            std::make_shared<GetOperator>(),
            std::make_shared<SetOperator>(),
            std::make_shared<DeleteOperator>(),
            std::make_shared<SyncDataOperator>(),
            std::make_shared<HealthCheckOperator>(),
        };
    }
};