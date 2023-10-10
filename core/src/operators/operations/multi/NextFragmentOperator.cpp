#include "NextFragmentOperator.h"

Response NextFragmentOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    uint64_t multiId = options.requestNumber;

    iterator_t senderIterator = dependencies.multipleResponses->getSenderIteratorByMultiId(multiId);
    dependencies.multipleResponses->markFragmentSend(multiId);

    if(senderIterator->hasNext()) {
        return Response::success(SimpleString<memDbDataLength_t>::fromVector(senderIterator->next()));
    } else {
        return Response::success();
    }
}

constexpr OperatorDescriptor NextFragmentOperator::desc() {
    return OperatorDescriptor{
        .type = OperatorType::NODE_MAINTENANCE,
        .number = OPERATOR_NUMBER,
        .name = "NEXT_FRAGMENT",
        .authorizedToExecute = { AuthenticationType::NODE },
    };
}