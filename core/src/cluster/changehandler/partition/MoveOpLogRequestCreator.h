#pragma once

#include "shared.h"
#include "messages/request/RequestBuilder.h"
#include "db/MapEntry.h"
#include "persistence/serializers/OperationLogSerializer.h"

struct CreateMoveOplogReqParams {
    std::vector<MapEntry<memDbDataLength_t>> oplog;
    bool applyNewOplog;
    bool clearOldOplog;
    int oldOplogId;
    int newOplogId;
};

class MoveOpLogRequestCreator {
public:
    MoveOpLogRequestCreator(const std::string& nodeAuthKey, memdbNodeId_t selfNodeId);

    Request create(CreateMoveOplogReqParams params);

private:
    OperationLogSerializer operationLogSerializer{};
    memdbNodeId_t selfNodeId;
    std::string nodeAuthKey;

    OperationBody convertMapEntryIntoOperation(const MapEntry<memDbDataLength_t>& entry);
};