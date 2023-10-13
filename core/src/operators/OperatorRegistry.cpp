#include "operators/OperatorRegistry.h"

OperatorRegistry::OperatorRegistry() {
    this->operators[OperatorNumbers::GET] = std::make_shared<GetOperator>();
    this->operators[OperatorNumbers::SET] = std::make_shared<SetOperator>();
    this->operators[OperatorNumbers::DEL] = std::make_shared<DeleteOperator>();
    this->operators[OperatorNumbers::HEALTH_CHECK] = std::make_shared<HealthCheckOperator>();
    this->operators[OperatorNumbers::SYNC_OPLOG] = std::make_shared<SyncOplogOperator>();
    this->operators[OperatorNumbers::MOVE_OPLOG] = std::make_shared<MovePartitionOplogOperator>();
    this->operators[OperatorNumbers::NEXT_FRAGMENT] = std::make_shared<NextFragmentOperator>();
    this->operators[OperatorNumbers::INIT_MULTI] = std::make_shared<InitMultiResponsesOperator>();
}

std::shared_ptr<Operator> OperatorRegistry::get(uint8_t operatorNumber) {
    std::map<uint8_t, std::shared_ptr<Operator>>::iterator iteratorFound = this->operators.find(operatorNumber);

    return iteratorFound != this->operators.end() ? iteratorFound->second : nullptr;
}
