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

            auto unsyncedOplog = this->cluster->syncOplog(lastTimestampProcessedFromOpLog[i], NodeGroupOptions{
                    .nodeGroupId = oplogId
            });


            this->applyOplog(unsyncedOplog, false);
            this->logger->info("Synchronized {0} oplog entries with the cluster", unsyncedOplog->size());
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
        return std::vector<uint64_t>{restoreSingleOplog()};
    }
}

std::vector<uint64_t> MemDb::restoreMultipleOplog() {
    uint32_t numberOplogs = this->operationLog->getNumberOplogFiles();
    std::vector<uint64_t> lastRestoredTimestamps{};

    for (uint32_t i = 0; i < numberOplogs; i++) {
        oplogSegmentIterator_t oplogIterator = this->operationLog->get(OperationLogOptions{
                .operationLogId = i
        });

        uint64_t lastAppliedTimestamp = this->applyOplog(oplogIterator, true);

        lastRestoredTimestamps.push_back(lastAppliedTimestamp);
    }

    return lastRestoredTimestamps;
}

uint64_t MemDb::restoreSingleOplog() {
    oplogSegmentIterator_t opLogsFromDisk = this->operationLog->get();

    uint64_t latestApplied = this->applyOplog(opLogsFromDisk, true);

    return latestApplied;
}

uint64_t MemDb::applyOplog(iterator_t oplogIterator, bool dontSaveInOperationLog) {
    OperationLogDeserializer operationLogDeserializer{};
    uint64_t latestTimestampApplied = 0;

    while(oplogIterator->hasNext()) {
        std::vector<uint8_t> serializedOplog = oplogIterator->next();
        std::vector<OperationBody> deserializedOplog = operationLogDeserializer.deserializeAll(serializedOplog);

        for(OperationBody& operationLogInDisk : deserializedOplog) {
            latestTimestampApplied = operationLogInDisk.timestamp;

            this->operatorDispatcher->executeOperation(
                    this->operatorRegistry->get(operationLogInDisk.operatorNumber),
                    operationLogInDisk,
                    OperationOptions{
                            .checkTimestamps = true,
                            .dontBroadcastToCluster = true,
                            .dontSaveInOperationLog = dontSaveInOperationLog,
                            .updateClockStrategy = LamportClock::UpdateClockStrategy::SET });
        }
    }

    this->operatorDispatcher->applyDelayedOperationsBuffer();

    return latestTimestampApplied;
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
    signal(SIGINT, &ctrlHandler);
#endif
}

void ctrlHandler(int s) {
#if __linux__
    exit(1);
#endif
}