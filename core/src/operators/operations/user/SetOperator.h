#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"
#include "operators/DbOperatorExecutor.h"

class SetOperator : public Operator {
public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x01;

    Response operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies dependencies) override {
        SimpleString key = operation.args->at(0);
        SimpleString value = operation.args->at(1);

        bool ignoreTimestmaps = !options.checkTimestamps;
        bool updated = dependencies.dbStore->put(key, value, ignoreTimestmaps, operation.timestamp, operation.nodeId);

        return updated ?
            Response::success() :
            Response::error(ErrorCode::ALREADY_REPLICATED);
    }

    std::vector<AuthenticationType> authorizedToExecute() override {
        return { AuthenticationType::NODE, AuthenticationType::API };
    }

    std::vector<OperatorDependency> dependencies() override {
        return { OperatorDependency::DB_STORE };
    }

    constexpr OperatorType type() override {
        return OperatorType::DB_STORE_WRITE;
    }

    constexpr uint8_t operatorNumber() override {
        return OPERATOR_NUMBER;
    }

    std::string name() override {
        return "SET";
    }
};