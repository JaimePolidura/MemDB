#pragma once

#include "Operator.h"
#include "OperatorDispatcher.h"

#include <string>

class GetOperator : public Operator {
public:
    static const uint8_t OPERATOR_NUMBER = 0x02;

    GetOperator() {
        STORE_OPERATOR(OPERATOR_NUMBER, std::shared_ptr<GetOperator>(this), operators);
    }

    std::shared_ptr<Response> operate(const OperationBody& operation, std::shared_ptr<Map> map) override {
        const std::string& key = std::string((char *) operation.args[0].arg, operation.args[0].lengthArg);

        std::optional<MapEntry> result = map->get(key);

        return result.has_value() ?
               std::make_shared<Response>(result.value().valueSize, result.value().value) :
               std::make_shared<Response>(0x02); //No successful
    }

    OperatorType type() override {
        return READ;
    }

    uint8_t operatorNumber() override {
        return OPERATOR_NUMBER;
    }
};