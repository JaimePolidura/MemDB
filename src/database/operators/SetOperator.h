#pragma once

#include "Operator.h"
#include "OperatorRegistry.h"

#include <string>

class SetOperator : public Operator {
public:
    SetOperator() {
        STORE_OPERATOR(dynamic_cast<SetOperator *>(this)->operatorNumber(), getInstance(), operators);
    }

    void operate(const OperationBody& operation, Map& map) override {
        const std::string& key = std::string((char *) operation.args[0].arg, operation.args[0].lengthArg);

        int valueSize = operation.args[1].lengthArg;
        uint8_t * valueValue = operation.args[1].arg;

        map.put(key, valueValue, valueSize);
    }

    OperatorType type() override {
        return WRITE;
    }

    uint8_t operatorNumber() override {
        return 0x01;
    }

private:
    static std::shared_ptr<SetOperator> instance;

    static std::shared_ptr<SetOperator> getInstance() {
        if(instance.get() != nullptr)
            instance = std::make_shared<SetOperator>();

        return instance;
    }
};