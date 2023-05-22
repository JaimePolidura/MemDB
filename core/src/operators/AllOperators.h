#pragma once

#include "shared.h"

#include "operators/Operator.h"
#include "operators/operations/user/GetOperator.h"
#include "operators/operations/user/SetOperator.h"
#include "operators/operations/user/DeleteOperator.h"
#include "operators/operations/cluster/HealthCheckOperator.h"
#include "operators/operations/user/SyncOplogOperator.h"

class AllOperators {
public:
    static std::vector<std::shared_ptr<Operator>> list() {
        return {
            std::make_shared<GetOperator>(),
            std::make_shared<SetOperator>(),
            std::make_shared<DeleteOperator>(),
            std::make_shared<SyncOplogOperator>(),
            std::make_shared<HealthCheckOperator>(),
        };
    }
};