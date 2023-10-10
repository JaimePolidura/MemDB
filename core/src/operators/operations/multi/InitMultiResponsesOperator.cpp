#include "InitMultiResponsesOperator.h"

Response InitMultiResponsesOperator::operate(const OperationBody &operation,
                                             const OperationOptions options,
                                             OperatorDependencies &dependencies) {
    uint8_t operatorNumber = operation.getArg(0).to<uint8_t>();
    iterator_t iterator = dependencies.getMultiResponseSenderIterator(operation, operatorNumber);
    uint64_t multiResponseId = options.requestNumber;

    dependencies.multipleResponses->registerIncomingMultiInit(multiResponseId, iterator);

    return Response::success(iterator->size());
}

constexpr OperatorDescriptor InitMultiResponsesOperator::desc() {
    return OperatorDescriptor{
        .type = OperatorType::NODE_MAINTENANCE,
        .number = OPERATOR_NUMBER,
        .name = "INIT_MULTI_RESPONSES",
        .authorizedToExecute = { AuthenticationType::NODE },
    };
}