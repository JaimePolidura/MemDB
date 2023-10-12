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
private:
    OperationLogSerializer operationLogSerializer{};
    std::string nodeAuthKey;

public:
    MoveOpLogRequestCreator(const std::string& nodeAuthKey);

    Request create(CreateMoveOplogReqParams params);
};