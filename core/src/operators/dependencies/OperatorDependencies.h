#pragma once

#include "persistence/oplog/OperationLog.h"
#include "utils/datastructures/map/Map.h"
#include "cluster/Cluster.h"

struct OperatorDependencies {
    memDbDataStore_t dbStore;
    operationLog_t operationLog;
    cluster_t cluster;
};