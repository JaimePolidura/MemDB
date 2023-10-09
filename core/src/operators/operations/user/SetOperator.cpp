#include "operators/operations/user/SetOperator.h"

Response SetOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    SimpleString key = operation.args->at(0);
    SimpleString value = operation.args->at(1);

    bool ignoreTimestamps = !options.checkTimestamps;
    bool updated = dependencies.dbStore->put(key, value, ignoreTimestamps, operation.timestamp, operation.nodeId);

    return updated ?
        Response::success() :
        Response::error(ErrorCode::ALREADY_REPLICATED);
}

constexpr OperatorDescriptor DeleteOperator::desc() {
    return OperatorDescriptor{
            .type = OperatorType::DB_STORE_WRITE,
            .number = OPERATOR_NUMBER,
            .name = "DELETE",
            .authorizedToExecute = { AuthenticationType::API, AuthenticationType::NODE },
    };
}