#include "MoveOpLogRequestCreator.h"

MoveOpLogRequestCreator::MoveOpLogRequestCreator(const std::string& nodeAuthKey): nodeAuthKey(nodeAuthKey) {}

Request MoveOpLogRequestCreator::create(CreateMoveOplogReqParams params) {
    std::vector<OperationBody> createOperations{params.oplog.size()};
    for(int i = 0; i < params.oplog.size(); i++){
        createOperations[i] = RequestBuilder::builder()
                .operatorNumber(OperatorNumbers::SET)
                ->addArg(params.oplog.at(i).key)
                ->timestamp(params.oplog.at(i).timestamp.counter)
                ->buildOperationBody();
    }

    auto serialized = this->operationLogSerializer.serializeAll(createOperations);

    return RequestBuilder::builder()
            .authKey(this->nodeAuthKey)
            ->operatorNumber(OperatorNumbers::MOVE_OPLOG)
            ->operatorFlag1(params.applyNewOplog)
            ->operatorFlag2(params.clearOldOplog) //Clear old oplog
            ->args({
                           SimpleString<memDbDataLength_t>::fromNumber(params.newOplogId),
                           SimpleString<memDbDataLength_t>::fromNumber(params.oldOplogId),
                           SimpleString<memDbDataLength_t>::fromVector(serialized)
                   })
            ->build();
}
