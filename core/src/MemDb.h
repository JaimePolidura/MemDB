#pragma once

#include "server/TCPServer.h"
#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "persistence/OperationLogDiskLoader.h"
#include "replication/Replication.h"
#include "utils/clock/LamportClock.h"
#include "replication/ReplicationCreator.h"
#include "logging/Logger.h"

#include "memdbtypes.h"

class MemDb {
private:
    operationLogBuffer_t operationLogBuffer;
    operatorDispatcher_t operatorDispatcher;
    configuration_t configuration;
    replication_t replication;
    memDbDataStore_t dbMap;
    lamportClock_t clock;
    tcpServer_t tcpServer;
    logger_t logger;

public:
    MemDb(logger_t logger, memDbDataStore_t map, configuration_t configuration, operatorDispatcher_t operatorDispatcher, tcpServer_t tcpServer,
          lamportClock_t clock) : dbMap(map), configuration(configuration), tcpServer(tcpServer), operatorDispatcher(operatorDispatcher),
          clock(clock), logger(logger) {}

    void run() {
        uint64_t lastTimestampStored = this->restoreDataFromOplogFromDisk();

        this->tcpServer->run();

        if(this->configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_REPLICATION)){
            this->clock->nodeId = std::stoi(this->replication->getNodeId());

            this->syncOplogFromCluster(lastTimestampStored);
        }
    }

    uint64_t tick(uint64_t other) {
        return this->clock->tick(other);
    }

private:
    void syncOplogFromCluster(uint64_t lastTimestampProcessedFromOpLog) {
        this->logger->info("Synchronizing oplog with the cluster");

        auto unsyncedOpLogs = this->replication->getUnsyncedOplog(lastTimestampProcessedFromOpLog);
        this->applyUnsyncedOplogFromCluster(unsyncedOpLogs);
        this->logger->info("Synchronized oplog with the cluster");

        this->replication->setRunning();

        this->replication->setReloadUnsyncedOpsCallback([this](std::vector<OperationBody> unsyncedOperations){
            this->applyUnsyncedOplogFromCluster(unsyncedOperations);
            this->operatorDispatcher->applyReplicatedOperationBuffer();
        });
    }

    uint64_t restoreDataFromOplogFromDisk() {
        OperationLogDiskLoader loader{};
        auto opLogsFromDisk = loader.getAllAndSaveCompacted();

        this->logger->info("Applaying logs from disk...");
        this->applyUnsyncedOplogFromCluster(opLogsFromDisk);

        return !opLogsFromDisk.empty() ? opLogsFromDisk[opLogsFromDisk.size() - 1].timestamp : 0;
    }

    void applyUnsyncedOplogFromCluster(const std::vector<OperationBody>& opLogs) {
        for(const auto& operationLogInDisk : opLogs)
            this->operatorDispatcher->executeOperator(OperationOptions{.requestOfNodeToReplicate = false},
                                                      operationLogInDisk);
        this->operatorDispatcher->applyReplicatedOperationBuffer();
    }
};