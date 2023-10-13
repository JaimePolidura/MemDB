#pragma once

#include "shared.h"

#include "operators/Operator.h"
#include "operators/operations/user/GetOperator.h"
#include "operators/operations/user/SetOperator.h"
#include "operators/operations/user/DeleteOperator.h"
#include "operators/operations/cluster/HealthCheckOperator.h"
#include "operators/operations/cluster/SyncOplogOperator.h"
#include "operators/operations/multi/InitMultiResponsesOperator.h"
#include "operators/operations/multi/NextFragmentOperator.h"

class AllOperators {
public:
    static std::vector<std::shared_ptr<Operator>> list();
};