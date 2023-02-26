#pragma once

#include "server/TCPServer.h"
#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "persistence/OperationLogDiskLoader.h"
#include "replication/Replication.h"
#include "utils/clock/LamportClock.h"
#include "memdbtypes.h"

class MemDb {
private:
    operationLogBuffer_t operationLogBuffer;
    operatorDispatcher_t operatorDispatcher;
    replication_t replicationNode;
    configuration_t configuration;
    lamportClock_t clock;
    tcpServer_t tcpServer;
    memDbDataStore_t dbMap;

public:
    MemDb(memDbDataStore_t map, configuration_t configuration, operatorDispatcher_t operatorDispatcher,
          tcpServer_t tcpServer, replication_t replicationNode, lamportClock_t clock)
            : dbMap(map), configuration(configuration), tcpServer(tcpServer), operatorDispatcher(operatorDispatcher),
            replicationNode(replicationNode), clock(clock)
          {}

    void run() {
        uint64_t lastTimestampStored = this->restoreDataFromOplog();
        if(this->configuration->getBoolean(ConfigurationKeys::USE_REPLICATION)){
            this->replicationNode->setup(lastTimestampStored);
            clock->nodeId = this->replicationNode->getNodeId();
        }

        this->tcpServer->run();
    }

    uint64_t tick(uint64_t other) {
        return this->replicationNode->tick(other);
    }

private:
    uint64_t restoreDataFromOplog() {
        OperationLogDiskLoader loader{};
        auto opLogsFromDisk = loader.getAllAndSaveCompacted(this->dbMap);

        printf("[SERVER] Applaying logs...\n");
        for(const auto& operationLogInDisk : opLogsFromDisk)
            this->operatorDispatcher->executeOperator(this->dbMap, OperationOptions{.requestFromReplication = false}, operationLogInDisk);

        return !opLogsFromDisk.empty() ? opLogsFromDisk[opLogsFromDisk.size() - 1].timestamp : 0;
    }
};