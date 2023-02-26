#pragma once

#include <memory>

#include "MemDb.h"
#include "config/keys/ConfigurationKeys.h"
#include "replication/Replication.h"
#include "auth/Authenticator.h"
#include "config/ConfigurationLoader.h"

class MemDbCreator {
public:
    static std::shared_ptr<MemDb> create() {
        configuration_t configuration = ConfiguartionLoader::load();
        lamportClock_t clock = std::make_shared<LamportClock>(1);
        replication_t replicationNode = std::make_shared<Replication>(clock, configuration);
        operationLogBuffer_t operationLogBuffer = std::make_shared<OperationLogBuffer>(configuration);
        memDbDataStore_t map = std::make_shared<Map<defaultMemDbSize_t>>(configuration->get<uint16_t>(ConfigurationKeys::NUMBER_BUCKETS));
        operatorDispatcher_t operatorDispatcher = std::make_shared<OperatorDispatcher>(map, clock, operationLogBuffer);
        tcpServer_t tcpServer = std::make_shared<TCPServer>(configuration, Authenticator{configuration}, operatorDispatcher);

        return std::make_shared<MemDb>(map, configuration, operatorDispatcher, tcpServer, replicationNode, clock);
    }
};