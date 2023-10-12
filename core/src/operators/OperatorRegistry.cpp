#include "operators/OperatorRegistry.h"

OperatorRegistry::OperatorRegistry() {
    std::vector<std::shared_ptr<Operator>> allOperators = AllOperators::list();

    for (const std::shared_ptr<Operator> &item: allOperators) {
        this->operators.insert(std::make_pair(item->desc().number, item));
    }
}

std::shared_ptr<Operator> OperatorRegistry::get(uint8_t operatorNumber) {
    std::map<uint8_t, std::shared_ptr<Operator>>::iterator iteratorFound = this->operators.find(operatorNumber);

    return iteratorFound != this->operators.end() ? iteratorFound->second : nullptr;
}
