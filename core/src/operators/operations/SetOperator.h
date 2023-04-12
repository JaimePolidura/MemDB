#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"
#include "operators/DbOperatorExecutor.h"

class SetOperator : public Operator, public DbOperatorExecutor {
public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x01;

    Response operate(const OperationBody& operation, const OperationOptions& options, memDbDataStore_t map) override {
        SimpleString key = operation.args->at(0);
        SimpleString value = operation.args->at(1);

        bool ignoreTimestmaps = !options.requestOfNodeToReplicate;
        bool updated = map->put(key, value, ignoreTimestmaps, operation.timestamp, operation.nodeId);

        return updated ?
            Response::success() :
            Response::error(ErrorCode::ALREADY_REPLICATED);
    }

    std::vector<AuthenticationType> authorizedToExecute() override {
        return { AuthenticationType::NODE, AuthenticationType::API };
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