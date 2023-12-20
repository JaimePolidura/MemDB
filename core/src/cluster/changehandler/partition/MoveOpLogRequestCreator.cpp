#include "MoveOpLogRequestCreator.h"

MoveOpLogRequestCreator::MoveOpLogRequestCreator(const std::string& nodeAuthKey, memdbNodeId_t selfNodeId):
    nodeAuthKey(nodeAuthKey), selfNodeId(selfNodeId) {}

Request MoveOpLogRequestCreator::create(CreateMoveOplogReqParams params) {
    std::vector<OperationBody> createOperations{params.oplog.size()};
    for(int i = 0; i < params.oplog.size(); i++){
        createOperations[i] = convertMapEntryIntoOperation(params.oplog.at(i));
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

OperationBody MoveOpLogRequestCreator::convertMapEntryIntoOperation(const MapEntry<memDbDataLength_t>& entry) {
    if(entry.type == NodeType::DATA) {
        std::shared_ptr<DataAVLNode<memDbDataLength_t>> dataEntry = entry.toData();
        return RequestBuilder::builder()
                .operatorNumber(OperatorNumbers::SET)
                ->addArg(entry.key)
                ->addArg(dataEntry->value)
                ->timestamp(dataEntry->timestamp.counter)
                ->buildOperationBody();
    } else if (entry.type == NodeType::COUNTER) {
        //TODO may return operator REPLICATE_COUNTER?
        return RequestBuilder::builder()
                .operatorNumber(OperatorNumbers::SET)
                ->buildOperationBody();
    }

    throw std::runtime_error("Invliad avl node type " + entry.type);
}