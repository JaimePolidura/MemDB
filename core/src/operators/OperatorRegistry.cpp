#include "operators/OperatorRegistry.h"

OperatorRegistry::OperatorRegistry() {
    this->operators[OperatorNumbers::GET] = std::make_shared<GetOperator>();
    this->operators[OperatorNumbers::SET] = std::make_shared<SetOperator>();
    this->operators[OperatorNumbers::DEL] = std::make_shared<DeleteOperator>();
    this->operators[OperatorNumbers::HEALTH_CHECK] = std::make_shared<HealthCheckOperator>();
    this->operators[OperatorNumbers::SYNC_OPLOG] = std::make_shared<SyncOplogOperator>();
    this->operators[OperatorNumbers::MOVE_OPLOG] = std::make_shared<MovePartitionOplogOperator>();
    this->operators[OperatorNumbers::NEXT_SYNC_OPLOG_SEGMENT] = std::make_shared<NextSegmentOplogOperator>();
    this->operators[OperatorNumbers::NEXT_SYNC_OPLOG_SEGMENT] = std::make_shared<NextSegmentOplogOperator>();
    this->operators[OperatorNumbers::FIX_OPLOG_SEGMENT] = std::make_shared<FixOplogSegmentOperator>();
    this->operators[OperatorNumbers::CAS] = std::make_shared<CasOperator>();
    this->operators[OperatorNumbers::CAS_PREPARE] = std::make_shared<PrepareCasOperator>();
}

std::shared_ptr<Operator> OperatorRegistry::get(uint8_t operatorNumber) {
    std::map<uint8_t, std::shared_ptr<Operator>>::iterator iteratorFound = this->operators.find(operatorNumber);

    return iteratorFound != this->operators.end() ? iteratorFound->second : nullptr;
}
