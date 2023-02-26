#pragma once

#include "server/TCPServer.h"
#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "persistence/OperationLogDiskLoader.h"
#include "replication/ReplicationNode.h"
#include "utils/clock/LamportClock.h"
#include "memdbtypes.h"

#include <memory>

class MemDb {
private:
    std::shared_ptr<OperationLogBuffer> operationLogBuffer;
    operatorDisptacher_t operatorDispatcher;
    std::shared_ptr<ReplicationNode> replicationNode;
    std::shared_ptr<Configuration> configuration;
    std::shared_ptr<LamportClock> clock;
    std::shared_ptr<TCPServer> tcpServer;
    memDbDataStore_t dbMap;

public:
    MemDb(memDbDataStore_t map, std::shared_ptr<Configuration> configuration, operatorDisptacher_t operatorDispatcher,
          std::shared_ptr<TCPServer> tcpServer, std::shared_ptr<ReplicationNode> replicationNode, std::shared_ptr<LamportClock> clock)
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