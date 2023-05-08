#pragma once

#include "server/TCPServer.h"
#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "cluster/Cluster.h"
#include "utils/clock/LamportClock.h"
#include "cluster/setup/ClusterCreator.h"
#include "logging/Logger.h"
#include "persistence/oplog/SingleOperationLog.h"

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
          lamportClock_t clock, cluster_t cluster, operationLog_t operationLog) : dbMap(map), configuration(configuration), tcpServer(tcpServer),
                                                                                      operatorDispatcher(operatorDispatcher), clock(clock), logger(logger), cluster(cluster), operationLog(operationLog),
                                                                                      operatorRegistry(std::make_shared<OperatorRegistry>()) {}

    void run() {
        std::vector<uint64_t> lastTimestampStored = this->restoreDataFromOplogFromDisk();

        if(this->configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_REPLICATION)){
            this->clock->nodeId = this->configuration->get<memdbNodeId_t>(ConfigurationKeys::MEMDB_CORE_NODE_ID);

            std::async(std::launch::async, [this, lastTimestampStored] () -> void {
                this->syncOplogFromCluster(lastTimestampStored);
            });
        }

        this->tcpServer->run();
    }

    uint64_t tick(uint64_t other) {
        return this->clock->tick(other);
    }

private:
    void syncOplogFromCluster(std::vector<uint64_t> lastTimestampProcessedFromOpLog) {
        this->logger->info("Synchronizing oplog with the cluster");

        for(int i = 0; i < lastTimestampProcessedFromOpLog.size(); i++){
            int oplogId = i;

            std::vector<OperationBody> unsyncedOplog = this->cluster->getUnsyncedOplog(lastTimestampProcessedFromOpLog[i], NodeGroupOptions{
                .nodeGroupId = oplogId
            });

            this->applyUnsyncedOplogFromCluster(unsyncedOplog, false);
            this->logger->info("Synchronized {0} oplog entries with the cluster", unsyncedOplog.size());
        }

        this->cluster->setRunning();
    }

    std::vector<uint64_t> restoreDataFromOplogFromDisk() {
        this->logger->info("Applying logs from disk...");

        bool usingReplication = this->configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_REPLICATION);
        bool usingPartitions = this->configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_PARTITIONS);

        if(usingReplication && usingPartitions){
            return restoreMultipleOplog();
        }else{
            return restoreSingleOplog();
        }
    }

    std::vector<uint64_t> restoreMultipleOplog() {
        int numberOplogs = this->operationLog->getNumberOplogFiles();
        std::vector<uint64_t> lastRestoredTimestamps{};

        for (int i = 0; i < numberOplogs; i++) {
            std::vector<OperationBody> oplog = this->operationLog->getFromDisk(OperationLogOptions{
                .operationLogId = i
            });

            this->applyUnsyncedOplogFromCluster(oplog, true);
            lastRestoredTimestamps.push_back(!oplog.empty() ? oplog[oplog.size() - 1].timestamp : 0);
        }

        return lastRestoredTimestamps;
    }

    std::vector<uint64_t> restoreSingleOplog() {
        std::vector<OperationBody> opLogsFromDisk = this->operationLog->getFromDisk();

        this->applyUnsyncedOplogFromCluster(opLogsFromDisk, true);

        return std::vector<uint64_t>{!opLogsFromDisk.empty() ? opLogsFromDisk[opLogsFromDisk.size() - 1].timestamp : 0};
    }

    void applyUnsyncedOplogFromCluster(const std::vector<OperationBody>& opLogs, bool dontSaveInOperationLog) {
        for(const auto& operationLogInDisk : opLogs)
            this->operatorDispatcher->executeOperation(
                    this->operatorRegistry->get(operationLogInDisk.operatorNumber),
                    operationLogInDisk,
                    OperationOptions{.checkTimestamps = true, .dontBroadcastToCluster = true, .dontSaveInOperationLog = dontSaveInOperationLog});

        this->operatorDispatcher->applyReplicatedOperationBuffer();
    }
};