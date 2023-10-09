#include "operators/operations/user/DeleteOperator.h"

Response DeleteOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    bool ignoreTimestamps = !options.checkTimestamps;
    bool removed = dependencies.dbStore->remove(operation.args->at(0), ignoreTimestamps, operation.timestamp, operation.nodeId);

    return removed ? Response::success() : Response::error(ErrorCode::UNKNOWN_KEY);
}

constexpr OperatorDescriptor DeleteOperator::desc() {
    return OperatorDescriptor{
        .type = OperatorType::DB_STORE_WRITE,
        .number = OPERATOR_NUMBER,
        .name = "SET",
        .authorizedToExecute = { AuthenticationType::API, AuthenticationType::NODE },
    };
}
