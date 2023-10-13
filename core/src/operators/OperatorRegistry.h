#pragma once

#include "shared.h"

#include "operators/Operator.h"
#include "OperatorNumbers.h"
#include "operators/operations/user/GetOperator.h"
#include "operators/operations/user/SetOperator.h"
#include "operators/operations/user/DeleteOperator.h"
#include "operators/operations/cluster/HealthCheckOperator.h"
#include "operators/operations/cluster/SyncOplogOperator.h"
#include "operators/operations/cluster/MovePartitionOplogOperator.h"
#include "operators/operations/multi/InitMultiResponsesOperator.h"
#include "operators/operations/multi/NextFragmentOperator.h"

class OperatorRegistry {
private:
    std::map<uint8_t, std::shared_ptr<Operator>> operators{};

public:
    OperatorRegistry();

    std::shared_ptr<Operator> get(uint8_t operatorNumber);
};

using operatorRegistry_t = std::shared_ptr<OperatorRegistry>;