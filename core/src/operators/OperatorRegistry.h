#pragma once

#include "shared.h"

#include "operators/Operator.h"
#include "operators/AllOperators.h"

class OperatorRegistry {
private:
    std::map<uint8_t, std::shared_ptr<Operator>> operators;

public:
    OperatorRegistry() {
        std::vector<std::shared_ptr<Operator>> allOperators = AllOperators::list();

        for (const std::shared_ptr<Operator> &item: allOperators)
            this->operators.insert(std::make_pair(item->operatorNumber(), item));
    }

    virtual ~OperatorRegistry() = default;

    virtual std::shared_ptr<Operator> get(uint8_t operatorNumber) {
        std::map<uint8_t, std::shared_ptr<Operator>>::iterator iteratorFound = this->operators.find(operatorNumber);

        return iteratorFound != this->operators.end() ? iteratorFound->second : nullptr;
    }
};

using operatorRegistry_t = std::shared_ptr<OperatorRegistry>;