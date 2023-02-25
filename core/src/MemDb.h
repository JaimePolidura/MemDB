#pragma once

#include "server/TCPServer.h"
#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "persistence/OperationLogDiskLoader.h"
#include "replication/ReplicationNode.h"
#include "utils/clock/LamportClock.h"

#include <memory>

class MemDb {
private:
    std::shared_ptr<OperationLogBuffer> operationLogBuffer;
    std::shared_ptr<OperatorDispatcher> operatorDispatcher;
    std::shared_ptr<ReplicationNode> replicationNode;
    std::shared_ptr<Configuration> configuration;
    std::shared_ptr<LamportClock> clock;
    std::shared_ptr<TCPServer> tcpServer;
    std::shared_ptr<Map> dbMap;

public:
    MemDb(std::shared_ptr<Map> map, std::shared_ptr<Configuration> configuration, std::shared_ptr<OperatorDispatcher> operatorDispatcher,
          std::shared_ptr<TCPServer> tcpServer, std::shared_ptr<ReplicationNode> replicationNode, std::shared_ptr<LamportClock> clock)
            : dbMap(map), configuration(configuration), tcpServer(tcpServer), operatorDispatcher(operatorDispatcher),
            replicationNode(replicationNode), clock(clock)
          {}

    void run() {
        if(this->configuration->getBoolean(ConfigurationKeys::USE_REPLICATION)){
            this->replicationNode->setup();
            clock->nodeId = this->replicationNode->getNodeId();
        }

        this->restoreDataFromOplog();
        this->tcpServer->run();
    }

    uint64_t tick(uint64_t other) {
        return this->replicationNode->tick(other);
    }

private:
    void restoreDataFromOplog() {
        OperationLogDiskLoader loader(this->operatorDispatcher);
        loader.loadIntoMapDb(this->dbMap);
    }
};