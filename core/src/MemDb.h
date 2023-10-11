#pragma once

#include "server/TCPServer.h"
#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "cluster/Cluster.h"
#include "utils/clock/LamportClock.h"
#include "cluster/setup/ClusterCreator.h"
#include "logging/Logger.h"
#include "persistence/SingleOperationLog.h"
#include "persistence/serializers/OperationLogDeserializer.h"
#include "db/MemDbStores.h"

#include "memdbtypes.h"

class MemDb {
private:
    operatorDispatcher_t operatorDispatcher;
    operatorRegistry_t operatorRegistry;
    configuration_t configuration;
    operationLog_t operationLog;
    cluster_t cluster;
    memDbStores_t dbStores;
    tcpServer_t tcpServer;
    lamportClock_t clock;
    logger_t logger;

public:
    MemDb(logger_t logger, memDbStores_t dbMaps, configuration_t configuration, operatorDispatcher_t operatorDispatcher, tcpServer_t tcpServer,
          lamportClock_t clock, cluster_t cluster, operationLog_t operationLog);

    void run();

private:
    void syncOplogFromCluster(std::vector<uint64_t> lastTimestampProcessedFromOpLog);

    std::vector<uint64_t> restoreDataFromOplogFromDisk();

    std::vector<uint64_t> restoreMultipleOplog();

    uint64_t restoreSingleOplog();

    uint64_t applyOplog(iterator_t oplogIterator, bool dontSaveInOperationLog, uint32_t partitionId);
};