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
        bytesDiskIterator_t oplogIterator = this->operationLog->getAll(OperationLogOptions{
                .operationLogId = i,
                .compressed = false,
        });

        uint64_t lastAppliedTimestamp = this->applyOplog(oplogIterator, false, i, OperationLogIteratorOrigin::LOCAL);

        lastRestoredTimestamps.push_back(lastAppliedTimestamp);
    }

    return lastRestoredTimestamps;
}

void MemDb::syncOplogFromCluster(std::vector<uint64_t> lastTimestampProcessedFromOpLog) {
    std::vector<std::future<void>> syncOplogFutures{};

    for(int i = 0; i < lastTimestampProcessedFromOpLog.size(); i++){
        std::future<void> syncOplogFuture = std::async(std::launch::async, [this, lastTimestampProcessedFromOpLog, i]() -> void {
            try {
                int oplogId = i;

                auto unsyncedOplog = this->cluster->syncOplog(lastTimestampProcessedFromOpLog[i], NodePartitionOptions{
                        .partitionId = oplogId
                });

                this->applyOplog(unsyncedOplog, true, oplogId, OperationLogIteratorOrigin::OTHER_NODE);
                this->logger->info("Synchronized {0} oplog entries with the cluster", unsyncedOplog->totalSize());
            }catch (const std::exception& e) {
                std::cout << e.what() << std::endl;
            }
        });

        syncOplogFutures.push_back(std::move(syncOplogFuture));
    }

    for (const std::future<void>& future : syncOplogFutures) {
        future.wait();
    }

    this->cluster->setRunning();
}

uint64_t MemDb::applyOplog(bytesDiskIterator_t oplogIterator, bool saveInOperationLog, uint32_t partitionId, OperationLogIteratorOrigin oplogOrigin) {
    OperationLogDeserializer operationLogDeserializer{};
    uint64_t latestTimestampApplied = 0;
    uint64_t numberOplogsApplied = 0;

    while(oplogIterator->hasNext()) {
        std::vector<uint8_t> serializedOplog = this->getNextOplogOrTryFix(oplogIterator, oplogOrigin);
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
                            .dontSaveInOperationLog = !saveInOperationLog,
                            .dontDebugLog = true,
                            .updateClockStrategy = LamportClock::UpdateClockStrategy::SET,
                            .partitionId = partitionId });
        }
    }

    this->logger->info("Applied {0} number of entries of oplog {1}", numberOplogsApplied, partitionId);

    this->operatorDispatcher->applyDelayedOperationsBuffer();

    return latestTimestampApplied;
}

std::vector<uint8_t> MemDb::getNextOplogOrTryFix(bytesDiskIterator_t oplogIterator, OperationLogIteratorOrigin oplogOrigin) {
    std::result<std::vector<uint8_t>> serializedOplogResult = oplogIterator->next();

    if(serializedOplogResult.has_error() && oplogOrigin == OperationLogIteratorOrigin::LOCAL){ //Maybe corrupted
        return this->tryFixLocalOplogSegment(oplogIterator);
    } else if(serializedOplogResult.has_error()) { //Maybe corrupted or eof
        return std::vector<uint8_t>{};
    } else {
        return serializedOplogResult.get();
    }
}

std::vector<uint8_t> MemDb::tryFixLocalOplogSegment(bytesDiskIterator_t oplogIterator) {
    if(!this->configuration->getBoolean(ConfigurationKeys::USE_REPLICATION)){
        this->logger->error("Detected corrupted oplog segment. CRCs don't match");
        return {};
    }

    oplogIterator_t oplogIteratorCasted = std::dynamic_pointer_cast<OplogIterator>(oplogIterator);
    OplogIndexSegmentDescriptor corruptedDescriptor = oplogIteratorCasted->getLastDescriptor();

    this->logger->error("Detected corrupted oplog segment from timestamp {0} to {1} in oplogId {2}. CRCs don't match",
                        corruptedDescriptor.min, corruptedDescriptor.max, oplogIteratorCasted->getOplogId());

    std::result<Response> responseOptional = this->cluster->fixOplogSegment(oplogIteratorCasted->getOplogId(),
        corruptedDescriptor.min, corruptedDescriptor.max);

    if(responseOptional.has_error() || !responseOptional->isSuccessful){
        this->logger->error("Impossible to get available node to fix oplog from timestamp {0} to {1} in oplogId {2}",
                            corruptedDescriptor.min, corruptedDescriptor.max, oplogIteratorCasted->getOplogId());
        return {};
    }

    uint32_t uncompressedSize = responseOptional->getResponseValueAtOffset(0, sizeof(uint32_t))
            .to<uint32_t>();
    std::vector<uint8_t> fixedOplog = responseOptional->getResponseValueAtOffset(sizeof(uint32_t), responseOptional->responseValue.size - sizeof(uint32_t))
            .toVector();

    this->operationLog->updateCorrupted(fixedOplog, uncompressedSize, oplogIteratorCasted->getLastSegmentOplogDescriptorDiskPtr());

    return this->compressor.uncompressBytes(fixedOplog, uncompressedSize)
            .get_or_throw_with([](auto ec){return "Impossible to decompress oplog in MemDb::tryFixLocalOplogSegment. Error code: " + ec;});
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