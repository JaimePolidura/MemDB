#pragma once

#include "server/TCPServer.h"
#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "persistence/OperationLogDiskLoader.h"
#include "replication/Replication.h"
#include "utils/clock/LamportClock.h"
#include "memdbtypes.h"
#include "replication/ReplicationCreator.h"

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

        this->tcpServer->run();

        if(this->configuration->getBoolean(ConfigurationKeys::USE_REPLICATION)){
            this->setupReplicationNode(lastTimestampStored);
        }
    }

    uint64_t tick(uint64_t other) {
        return this->clock->tick(other);
    }

private:
    void setupReplicationNode(uint64_t lastTimestampProcessedFromOpLog) {
        this->clock->nodeId = this->replication->getNodeId();

        auto unsyncedOpLogs = this->replication->getUnsyncedOpLogs(lastTimestampProcessedFromOpLog);
        this->applyOperationLogs(unsyncedOpLogs);

        this->operatorDispatcher->applyReplicatedOperationBuffer();
        this->replication->setRunning();

        this->replication->setReloadUnsyncedOpsCallback([this](std::vector<OperationBody> unsyncedOperations){
            this->applyOperationLogs(unsyncedOperations);
            this->operatorDispatcher->applyReplicatedOperationBuffer();
        });
    }

    uint64_t restoreDataFromOplog() {
        OperationLogDiskLoader loader{};
        auto opLogsFromDisk = loader.getAllAndSaveCompacted(this->dbMap);

        printf("[SERVER] Applaying logs from disk...\n");
        this->applyOperationLogs(opLogsFromDisk);

        return !opLogsFromDisk.empty() ? opLogsFromDisk[opLogsFromDisk.size() - 1].timestamp : 0;
    }

    void applyOperationLogs(const std::vector<OperationBody>& opLogs) {
        for(const auto& operationLogInDisk : opLogs)
            this->operatorDispatcher->executeOperator(this->dbMap, OperationOptions{.requestFromReplication = false}, operationLogInDisk);
    }
};