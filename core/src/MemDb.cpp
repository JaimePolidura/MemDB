#include "MemDb.h"

void listenCtrlCSignal(operationLog_t operationLog);
void ctrlHandler(int s);

MemDb::MemDb(logger_t logger, memDbDataStore_t map, configuration_t configuration, operatorDispatcher_t operatorDispatcher, tcpServer_t tcpServer,
    lamportClock_t clock, cluster_t cluster, operationLog_t operationLog) : dbMap(map), configuration(configuration), tcpServer(tcpServer),
    operatorDispatcher(operatorDispatcher), clock(clock), logger(logger), cluster(cluster), operationLog(operationLog),
    operatorRegistry(std::make_shared<OperatorRegistry>()) {}

void MemDb::run() {
    std::vector<uint64_t> lastTimestampStored = this->restoreDataFromOplogFromDisk();

    if(this->configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_REPLICATION)){
        this->clock->nodeId = this->configuration->get<memdbNodeId_t>(ConfigurationKeys::MEMDB_CORE_NODE_ID);

        this->syncOplogFromCluster(lastTimestampStored);
    }

    this->tcpServer->run();

    listenCtrlCSignal(this->operationLog);
}

void MemDb::syncOplogFromCluster(std::vector<uint64_t> lastTimestampProcessedFromOpLog) {
    std::vector<std::future<void>> syncOplogFutures{};

    for(int i = 0; i < lastTimestampProcessedFromOpLog.size(); i++){
        std::future<void> syncOplogFuture = std::async(std::launch::async, [this, lastTimestampProcessedFromOpLog, i]() -> void{
            int oplogId = i;

            std::vector<OperationBody> unsyncedOplog = this->cluster->getUnsyncedOplog(lastTimestampProcessedFromOpLog[i], NodeGroupOptions{
                    .nodeGroupId = oplogId
            });

            this->applyUnsyncedOplogFromCluster(unsyncedOplog, false);
            this->logger->info("Synchronized {0} oplog entries with the cluster", unsyncedOplog.size());
        });

        syncOplogFutures.push_back(std::move(syncOplogFuture));
    }

    for (const std::future<void>& future : syncOplogFutures)
        future.wait();

    this->cluster->setRunning();
}

std::vector<uint64_t> MemDb::restoreDataFromOplogFromDisk() {
    this->logger->info("Applying logs from disk...");

    bool usingReplication = this->configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_REPLICATION);
    bool usingPartitions = this->configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_PARTITIONS);

    if(usingReplication && usingPartitions){
        return restoreMultipleOplog();
    }else{
        return restoreSingleOplog();
    }
}

std::vector<uint64_t> MemDb::restoreMultipleOplog() {
    uint32_t numberOplogs = this->operationLog->getNumberOplogFiles();
    std::vector<uint64_t> lastRestoredTimestamps{};

    for (uint32_t i = 0; i < numberOplogs; i++) {
        std::vector<OperationBody> oplog = this->operationLog->get(OperationLogOptions{
                .operationLogId = i
        });

        this->applyUnsyncedOplogFromCluster(oplog, true);
        lastRestoredTimestamps.push_back(!oplog.empty() ? oplog[oplog.size() - 1].timestamp : 0);
    }

    return lastRestoredTimestamps;
}

std::vector<uint64_t> MemDb::restoreSingleOplog() {
    std::vector<OperationBody> opLogsFromDisk = this->operationLog->get();

    this->applyUnsyncedOplogFromCluster(opLogsFromDisk, true);

    return std::vector<uint64_t>{!opLogsFromDisk.empty() ? opLogsFromDisk[opLogsFromDisk.size() - 1].timestamp : 0};
}

void MemDb::applyUnsyncedOplogFromCluster(const std::vector<OperationBody>& opLogs, bool dontSaveInOperationLog) {
    for(const auto& operationLogInDisk : opLogs)
        this->operatorDispatcher->executeOperation(
                this->operatorRegistry->get(operationLogInDisk.operatorNumber),
                operationLogInDisk,
                OperationOptions{.checkTimestamps = true, .dontBroadcastToCluster = true, .dontSaveInOperationLog = dontSaveInOperationLog});

    this->operatorDispatcher->applyDelayedOperationsBuffer();
}

#ifdef __linux__
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#endif

operationLog_t globalOperationLog;

void listenCtrlCSignal(operationLog_t operationLog) {
    globalOperationLog = operationLog;

#ifdef __linux__
    sigaction(SIGINT, &ctrlHandler, NULL);
#endif
}

void ctrlHandler(int s) {
    globalOperationLog->flush();

#if __linux__
    exit(1);
#endif
}