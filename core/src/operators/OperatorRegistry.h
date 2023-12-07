#pragma once

#include "shared.h"

#include "operators/operations/syncoplog/NextSegmentOplogOperator.h"
#include "operators/operations/syncoplog/SyncOplogOperator.h"
#include "operators/operations/cluster/MovePartitionOplogOperator.h"
#include "operators/operations/cluster/FixOplogSegmentOperator.h"
#include "operators/operations/cluster/HealthCheckOperator.h"
#include "operators/operations/user/ContainsOperator.h"
#include "operators/operations/user/DeleteOperator.h"
#include "operators/operations/user/GetOperator.h"
#include "operators/operations/user/SetOperator.h"
#include "operators/operations/cas/CasOperator.h"
#include "operators/operations/cas/PrepareCasOperator.h"
#include "operators/operations/cluster/JoinClusterAnnounceOperator.h"
#include "operators/operations/cas/AcceptCasOperator.h"
#include "operators/operations/cluster/GetNodeDataOperator.h"
#include "operators/operations/cluster/GetClusterConfigOperator.h"
#include "operators/Operator.h"
#include "OperatorNumbers.h"
#include "operators/operations/cluster/DoLeaveClusterOperator.h"
#include "operators/operations/cluster/LeaveClusterAnnounceOperator.h"

class OperatorRegistry {
    std::map<uint8_t, std::shared_ptr<Operator>> operators{};

public:
    OperatorRegistry();

    std::shared_ptr<Operator> get(uint8_t operatorNumber);
};

using operatorRegistry_t = std::shared_ptr<OperatorRegistry>;