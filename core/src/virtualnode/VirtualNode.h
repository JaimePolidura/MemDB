#pragma once

#include "utils/datastructures/map/Map.h"
#include "memdbtypes.h"
#include "persistence/oplog/SingleOperationLog.h"

class VirtualNode {
private:
    singleOperationLog_t operationLog;
    memDbDataStore_t dataStore;
    int virtualNodeId;

};