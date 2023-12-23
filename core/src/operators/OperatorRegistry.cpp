#include "operators/OperatorRegistry.h"

OperatorRegistry::OperatorRegistry() {
    this->operators[OperatorNumbers::GET] = std::make_shared<GetOperator>();
    this->operators[OperatorNumbers::SET] = std::make_shared<SetOperator>();
    this->operators[OperatorNumbers::DEL] = std::make_shared<DeleteOperator>();
    this->operators[OperatorNumbers::CONTAINS] = std::make_shared<ContainsOperator>();

    this->operators[OperatorNumbers::UPDATE_COUNTER] = std::make_shared<UpdateCounterOperator>();
    this->operators[OperatorNumbers::REPLICATE_COUNTER] = std::make_shared<ReplicateCounterOperator>();

    this->operators[OperatorNumbers::HEALTH_CHECK] = std::make_shared<HealthCheckOperator>();
    this->operators[OperatorNumbers::SYNC_OPLOG] = std::make_shared<SyncOplogOperator>();
    this->operators[OperatorNumbers::MOVE_OPLOG] = std::make_shared<MovePartitionOplogOperator>();
    this->operators[OperatorNumbers::NEXT_SYNC_OPLOG_SEGMENT] = std::make_shared<NextSegmentOplogOperator>();
    this->operators[OperatorNumbers::FIX_OPLOG_SEGMENT] = std::make_shared<FixOplogSegmentOperator>();

    this->operators[OperatorNumbers::CAS] = std::make_shared<CasOperator>();
    this->operators[OperatorNumbers::CAS_PREPARE] = std::make_shared<PrepareCasOperator>();
    this->operators[OperatorNumbers::CAS_ACCEPT] = std::make_shared<AcceptCasOperator>();

    this->operators[OperatorNumbers::GET_CLUSTER_CONFIG] = std::make_shared<GetClusterConfigOperator>();
    this->operators[OperatorNumbers::JOIN_CLUSTER_ANNOUNCE] = std::make_shared<JoinClusterAnnounceOperator>();
    this->operators[OperatorNumbers::LEAVE_CLUSTER_ANNOUNCE] = std::make_shared<LeaveClusterAnnounceOperator>();
    this->operators[OperatorNumbers::DO_LEAVE_CLUSTER] = std::make_shared<DoLeaveClusterOperator>();
    this->operators[OperatorNumbers::GET_NODE_DATA] = std::make_shared<GetNodeDataOperator>();
}

std::shared_ptr<Operator> OperatorRegistry::get(uint8_t operatorNumber) {
    std::map<uint8_t, std::shared_ptr<Operator>>::iterator iteratorFound = this->operators.find(operatorNumber);

    return iteratorFound != this->operators.end() ? iteratorFound->second : nullptr;
}
