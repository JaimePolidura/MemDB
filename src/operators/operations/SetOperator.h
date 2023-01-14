#pragma once

#include "operators/Operator.h"

#include <string>

/**
 * flag1 set: key will be the hash
 */
class SetOperator : public Operator {
public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x01;

    Response operate(const OperationBody& operation, std::shared_ptr<Map> map) override {
        int valueSize = operation.args->at(1).lengthArg;
        uint8_t * valueValue = operation.args->at(1).arg;

        if(!operation.flag1){
            const std::string& key = std::string((char *) operation.args->at(0).arg, operation.args->at(0).lengthArg);
            map->put(key, valueValue, valueSize);
        }else{
            uint32_t keyHash = * operation.args->at(0).arg;
            map->putHash(keyHash, valueValue, valueSize);
        }

        return Response::success();
    }

    constexpr OperatorType type() override {
        return WRITE;
    }

    constexpr uint8_t operatorNumber() override {
        return OPERATOR_NUMBER;
    }
};