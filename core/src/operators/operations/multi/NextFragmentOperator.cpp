#include "NextFragmentOperator.h"

Response NextFragmentOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    uint64_t multiId = options.requestNumber;
    
    return Response::success();
}

constexpr OperatorDescriptor NextFragmentOperator::desc() {
    return OperatorDescriptor{
        .type = OperatorType::NODE_MAINTENANCE,
        .number = OPERATOR_NUMBER,
        .name = "NEXT_FRAGMENT",
        .authorizedToExecute = { AuthenticationType::NODE },
    };
}