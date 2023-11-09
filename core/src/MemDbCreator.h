#pragma once

#include <memory>

#include "MemDb.h"
#include "cluster/Cluster.h"
#include "config/ConfigurationLoader.h"
#include "cluster/setup/ClusterNodeSetup.h"
#include "cluster/changehandler/partition/PartitionClusterNodeChangeHandler.h"
#include "cluster/changehandler/simple/SimpleClusterNodeChangeHandler.h"
#include "cluster/setup/PartitionsClusterNodeSetup.h"
#include "cluster/setup/SimpleClusterNodeSetup.h"

#include "persistence/MultipleOperationLog.h"

class MemDbCreator {
public:
    static std::shared_ptr<MemDb> create(int nArgs, char ** args);

private:
    static operationLog_t createOperationLogObject(configuration_t configuration, logger_t logger);

    static cluster_t createClusterObject(logger_t logger, configuration_t configuration, onGoingSyncOplogs_t multipleResponses,
        memDbStores_t memDbStores, operatorDispatcher_t operatorDispatcher, operationLog_t operationLog);

    static void initializeOplog(configuration_t configuration, operationLog_t operationLog, cluster_t cluster);
};