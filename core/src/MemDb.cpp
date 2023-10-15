#include "MemDb.h"

void listenCtrlCSignal(operationLog_t operationLog);
void ctrlHandler(int s);

MemDb::MemDb(logger_t logger, memDbStores_t dbStores, configuration_t configuration, operatorDispatcher_t operatorDispatcher, tcpServer_t tcpServer,
    lamportClock_t clock, cluster_t cluster, operationLog_t operationLog) : dbStores(dbStores), configuration(configuration), tcpServer(tcpServer),
    operatorDispatcher(operatorDispatcher), clock(clock), logger(logger), cluster(cluster), operationLog(operationLog),
    operatorRegistry(std::make_shared<OperatorRegistry>()) {}

void MemDb::run() {
    std::vector<uint64_t> lastTimestampStored = this->restoreOplogFromDisk();

    if(this->configuration->getBoolean(ConfigurationKeys::USE_REPLICATION)){
        this->clock->nodeId = this->configuration->get<memdbNodeId_t>(ConfigurationKeys::NODE_ID);

        this->syncOplogFromCluster(lastTimestampStored);
    }

    this->tcpServer->run();

    listenCtrlCSignal(this->operationLog);
}

std::vector<uint64_t> MemDb::restoreOplogFromDisk() {
    this->logger->info("Restoring logs from disk...");

    uint32_t numberOplogs = this->operationLog->getNumberOplogFiles();
    std::vector<uint64_t> lastRestoredTimestamps{};

    for (uint32_t i = 0; i < numberOplogs; i++) {
        oplogSegmentIterator_t oplogIterator = this->operationLog->getAll(OperationLogOptions{
                .operationLogId = i
        });

        uint64_t lastAppliedTimestamp = this->applyOplog(oplogIterator, true, i);

        lastRestoredTimestamps.push_back(lastAppliedTimestamp);
    }

    return lastRestoredTimestamps;
}

void MemDb::syncOplogFromCluster(std::vector<uint64_t> lastTimestampProcessedFromOpLog) {
    std::vector<std::future<void>> syncOplogFutures{};

    for(int i = 0; i < lastTimestampProcessedFromOpLog.size(); i++){
        std::future<void> syncOplogFuture = std::async(std::launch::async, [this, lastTimestampProcessedFromOpLog, i]() -> void{
            int oplogId = i;

            auto unsyncedOplog = this->cluster->syncOplog(lastTimestampProcessedFromOpLog[i], NodeGroupOptions{
                    .nodeGroupId = oplogId
            });

            this->applyOplog(unsyncedOplog, false, oplogId);
            this->logger->info("Synchronized {0} oplog entries with the cluster", unsyncedOplog->totalSize());
        });

        syncOplogFutures.push_back(std::move(syncOplogFuture));
    }

    for (const std::future<void>& future : syncOplogFutures)
        future.wait();

    this->cluster->setRunning();
}

uint64_t MemDb::applyOplog(iterator_t<std::vector<uint8_t>> oplogIterator, bool dontSaveInOperationLog, uint32_t partitionId) {
    OperationLogDeserializer operationLogDeserializer{};
    uint64_t latestTimestampApplied = 0;
    uint64_t numberOplogsApplied = 0;

    while(oplogIterator->hasNext()) {
        std::vector<uint8_t> serializedOplog = oplogIterator->next();
        std::vector<OperationBody> deserializedOplog = operationLogDeserializer.deserializeAll(serializedOplog);

        for(OperationBody& operationLogInDisk : deserializedOplog) {
            latestTimestampApplied = operationLogInDisk.timestamp;
            numberOplogsApplied++;

            this->operatorDispatcher->executeOperation(
                    this->operatorRegistry->get(operationLogInDisk.operatorNumber),
                    operationLogInDisk,
                    OperationOptions{
                            .checkTimestamps = true,
                            .dontBroadcastToCluster = true,
                            .dontSaveInOperationLog = dontSaveInOperationLog,
                            .dontDebugLog = true,
                            .updateClockStrategy = LamportClock::UpdateClockStrategy::SET,
                            .partitionId = partitionId });
        }
    }

    this->logger->info("Applied {0} number of entries of oplog {1}", numberOplogsApplied, partitionId);

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