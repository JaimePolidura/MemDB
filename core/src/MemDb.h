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

        this->logger->info("Applaying logs from disk...");
        this->applyOperationLogs(opLogsFromDisk);

        return !opLogsFromDisk.empty() ? opLogsFromDisk[opLogsFromDisk.size() - 1].timestamp : 0;
    }

    void applyOperationLogs(const std::vector<OperationBody>& opLogs) {
        for(const auto& operationLogInDisk : opLogs)
            this->operatorDispatcher->executeOperator(OperationOptions{.requestFromReplication = false},
                                                      operationLogInDisk);
    }
};