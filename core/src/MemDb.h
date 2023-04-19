#pragma once

#include "server/TCPServer.h"
#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "replication/Replication.h"
#include "utils/clock/LamportClock.h"
#include "replication/ReplicationCreator.h"
#include "logging/Logger.h"
#include "persistence/OperationLog.h"

#include "memdbtypes.h"

class MemDb {
private:
    operatorDispatcher_t operatorDispatcher;
    operatorRegistry_t operatorRegistry;
    configuration_t configuration;
    operationLog_t operationLog;
    replication_t replication;
    memDbDataStore_t dbMap;
    tcpServer_t tcpServer;
    lamportClock_t clock;
    logger_t logger;

public:
    MemDb(logger_t logger, memDbDataStore_t map, configuration_t configuration, operatorDispatcher_t operatorDispatcher, tcpServer_t tcpServer,
          lamportClock_t clock, replication_t replication, operationLog_t operationLog) : dbMap(map), configuration(configuration), tcpServer(tcpServer),
          operatorDispatcher(operatorDispatcher), clock(clock), logger(logger), replication(replication), operationLog(operationLog),
          operatorRegistry(std::make_shared<OperatorRegistry>()) {}

    void run() {
        uint64_t lastTimestampStored = this->restoreDataFromOplogFromDisk();

        if(this->configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_REPLICATION)){
            this->clock->nodeId = this->replication->getNodeId();

            std::async(std::launch::async, [this, lastTimestampStored] -> void {
                this->syncOplogFromCluster(lastTimestampStored);
            });
        }

        this->tcpServer->run();
    }

    uint64_t tick(uint64_t other) {
        return this->clock->tick(other);
    }

private:
    void syncOplogFromCluster(uint64_t lastTimestampProcessedFromOpLog) {
        this->logger->info("Synchronizing oplog with the cluster");

        std::vector<OperationBody> unsyncedOplog = this->replication->getUnsyncedOplog(lastTimestampProcessedFromOpLog);
        this->applyUnsyncedOplogFromCluster(unsyncedOplog);
        this->logger->info("Synchronized {0} oplog entries with the cluster", unsyncedOplog.size());

        this->replication->setRunning();

        this->replication->setReloadUnsyncedOplogCallback([this](std::vector<OperationBody> unsyncedOperations) {
            this->applyUnsyncedOplogFromCluster(unsyncedOperations);
            this->operatorDispatcher->applyReplicatedOperationBuffer();
        });
    }

    uint64_t restoreDataFromOplogFromDisk() {
        std::vector<OperationBody> opLogsFromDisk = this->operationLog->getFromDisk();

        this->logger->info("Applaying logs from disk...");
        this->applyUnsyncedOplogFromCluster(opLogsFromDisk);

        return !opLogsFromDisk.empty() ? opLogsFromDisk[opLogsFromDisk.size() - 1].timestamp : 0;
    }

    void applyUnsyncedOplogFromCluster(const std::vector<OperationBody>& opLogs) {
        for(const auto& operationLogInDisk : opLogs)
            this->operatorDispatcher->executeOperator(
                    this->operatorRegistry->get(operationLogInDisk.operatorNumber),
                    operationLogInDisk,
                    OperationOptions{.requestOfNodeToReplicate = true});

        this->operatorDispatcher->applyReplicatedOperationBuffer();
    }
};