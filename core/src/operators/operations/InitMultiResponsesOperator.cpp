#include "InitMultiResponsesOperator.h"

Response InitMultiResponsesOperator::operate(const OperationBody &operation,
                                             const OperationOptions options,
                                             OperatorDependencies &dependencies) {
    dependencies.multipleResponses->handleInitMultiResponseRequest(options.requestNumber, operation);

    return Response::success();
}

constexpr OperatorDescriptor InitMultiResponsesOperator::desc() {
    return OperatorDescriptor{
        .type = OperatorType::NODE_MAINTENANCE,
        .number = OPERATOR_NUMBER,
        .name = "INIT_MULTI_RESPONSES",
        .authorizedToExecute = { AuthenticationType::NODE },
        .isMultiResponsesFragment = false,
    };
}