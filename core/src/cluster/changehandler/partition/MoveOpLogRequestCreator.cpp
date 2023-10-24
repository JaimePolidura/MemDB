#include "MoveOpLogRequestCreator.h"

MoveOpLogRequestCreator::MoveOpLogRequestCreator(const std::string& nodeAuthKey, memdbNodeId_t selfNodeId):
    nodeAuthKey(nodeAuthKey), selfNodeId(selfNodeId) {}

Request MoveOpLogRequestCreator::create(CreateMoveOplogReqParams params) {
    std::vector<OperationBody> createOperations{params.oplog.size()};
    for(int i = 0; i < params.oplog.size(); i++){
        createOperations[i] = RequestBuilder::builder()
                .operatorNumber(OperatorNumbers::SET)
                ->addArg(params.oplog.at(i).key)
                ->addArg(params.oplog.at(i).value)
                ->timestamp(params.oplog.at(i).timestamp.counter)
                ->buildOperationBody();
    }

    auto serialized = this->operationLogSerializer.serializeAll(createOperations);

    return RequestBuilder::builder()
            .authKey(this->nodeAuthKey)
            ->operatorNumber(OperatorNumbers::MOVE_OPLOG)
            ->operatorFlag1(params.applyNewOplog)
            ->operatorFlag2(params.clearOldOplog) //Clear old oplog
            ->selfNode(this->selfNodeId)
            ->args({
                SimpleString<uint32_t>::fromNumber(params.newOplogId),
                SimpleString<uint32_t>::fromNumber(params.oldOplogId),
                SimpleString<memDbDataLength_t>::fromVector(serialized)
            })
            ->build();
}
