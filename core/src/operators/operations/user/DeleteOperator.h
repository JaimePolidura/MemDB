#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"
#include "operators/DbOperatorExecutor.h"

class DeleteOperator : public Operator {
public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x03;

    Response operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies dependencies) override {
        bool ignoreTimestmaps = !options.checkTimestamps;
        bool removed = dependencies.dbStore->remove(operation.args->at(0), ignoreTimestmaps, operation.timestamp, operation.nodeId);

        return removed ? Response::success() : Response::error(ErrorCode::UNKNOWN_KEY);
    }

    std::vector<AuthenticationType> authorizedToExecute() override {
        return { AuthenticationType::API, AuthenticationType::NODE };
    }

    std::vector<OperatorDependency> dependencies() override {
        return { OperatorDependency::DB_STORE };
    }

    constexpr OperatorType type() override {
        return OperatorType::WRITE;
    }

    constexpr uint8_t operatorNumber() override {
        return OPERATOR_NUMBER;
    }

    std::string name() override {
        return "SET";
    }
};