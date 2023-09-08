#pragma once

#include <memory>

#include "MemDb.h"
#include "config/keys/ConfigurationKeys.h"
#include "cluster/Cluster.h"
#include "auth/Authenticator.h"
#include "config/ConfigurationLoader.h"

#include "persistence/oplog/MultipleOperationLog.h"
#include "persistence/oplog/SingleOperationLog.h"

class MemDbCreator {
public:
    static std::shared_ptr<MemDb> create(int nArgs, char ** args);

private:
    static operationLog_t createOperationLogObject(configuration_t configuration, cluster_t cluster);

    static cluster_t createClusterObject(logger_t logger, configuration_t configuration);

    static operationLog_t setupMultipleOplogConfiguration(configuration_t configuration, cluster_t cluster);

    static void setupClusterChangeWatcher(cluster_t cluster, operationLog_t operationLog, configuration_t configuration,
                                          logger_t logger, operatorDispatcher_t operatorDispatcher);
};