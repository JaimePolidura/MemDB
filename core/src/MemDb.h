#pragma once

#include "server/TCPServer.h"
#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "cluster/Cluster.h"
#include "utils/clock/LamportClock.h"
#include "cluster/setup/ClusterCreator.h"
#include "logging/Logger.h"
#include "persistence/SingleOperationLog.h"

#include "memdbtypes.h"

class MemDb {
private:
    operatorDispatcher_t operatorDispatcher;
    operatorRegistry_t operatorRegistry;
    configuration_t configuration;
    operationLog_t operationLog;
    cluster_t cluster;
    memDbDataStore_t dbMap;
    tcpServer_t tcpServer;
    lamportClock_t clock;
    logger_t logger;

public:
    MemDb(logger_t logger, memDbDataStore_t map, configuration_t configuration, operatorDispatcher_t operatorDispatcher, tcpServer_t tcpServer,
          lamportClock_t clock, cluster_t cluster, operationLog_t operationLog);

    void run();

private:
    void syncOplogFromCluster(std::vector<uint64_t> lastTimestampProcessedFromOpLog);

    std::vector<uint64_t> restoreDataFromOplogFromDisk();

    std::vector<uint64_t> restoreMultipleOplog();

    std::vector<uint64_t> restoreSingleOplog();

    void applyUnsyncedOplogFromCluster(std::vector<OperationBody>& opLogs, bool dontSaveInOperationLog);
};