#pragma once

#include "server/TCPServer.h"
#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "persistence/OperationLogDiskLoader.h"
#include "replication/Replication.h"
#include "utils/clock/LamportClock.h"
#include "memdbtypes.h"
#include "replication/ReplicationNodeStarter.h"

class MemDb {
private:
    operationLogBuffer_t operationLogBuffer;
    operatorDispatcher_t operatorDispatcher;
    replication_t replication;
    configuration_t configuration;
    lamportClock_t clock;
    tcpServer_t tcpServer;
    memDbDataStore_t dbMap;

public:
    MemDb(memDbDataStore_t map, configuration_t configuration, operatorDispatcher_t operatorDispatcher, tcpServer_t tcpServer,
          lamportClock_t clock) : dbMap(map), configuration(configuration), tcpServer(tcpServer), operatorDispatcher(operatorDispatcher), clock(clock)
          {}

    void run() {
        uint64_t lastTimestampStored = this->restoreDataFromOplog();
        if(this->configuration->getBoolean(ConfigurationKeys::USE_REPLICATION)){
            this->setupReplicationNode(lastTimestampStored);
        }

        this->tcpServer->run();
    }

    uint64_t tick(uint64_t other) {
        return this->clock->tick(other);
    }

private:
    void setupReplicationNode(uint64_t lastTimestampProcessedFromOpLog) {
        ReplicationNodeStarter replicationNodeStarter{this->configuration};

        this->replication = replicationNodeStarter.setup(lastTimestampProcessedFromOpLog);
        auto unsyncedOpLogs = replicationNodeStarter.getUnsyncedOpLogs(this->replication, lastTimestampProcessedFromOpLog);

        printf("[SERVER] Applaying unsynced logs from nodes...\n");
        this->applyAllLogs(unsyncedOpLogs);
        this->clock->nodeId = this->replication->getNodeId();
    }

    uint64_t restoreDataFromOplog() {
        OperationLogDiskLoader loader{};
        auto opLogsFromDisk = loader.getAllAndSaveCompacted(this->dbMap);

        printf("[SERVER] Applaying logs from disk...\n");
        this->applyAllLogs(opLogsFromDisk);

        return !opLogsFromDisk.empty() ? opLogsFromDisk[opLogsFromDisk.size() - 1].timestamp : 0;
    }

    void applyAllLogs(const std::vector<OperationBody>& opLogs) {
        for(const auto& operationLogInDisk : opLogs)
            this->operatorDispatcher->executeOperator(this->dbMap, OperationOptions{.requestFromReplication = false}, operationLogInDisk);
    }
};