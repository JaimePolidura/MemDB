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

        logger_t logger = std::make_shared<Logger>("Starting memdb");
        replication_t replication = createReplicationObject(logger, configuration);
        lamportClock_t clock = std::make_shared<LamportClock>(1);
        operationLogBuffer_t operationLogBuffer = std::make_shared<OperationLogBuffer>(configuration);
        memDbDataStore_t map = std::make_shared<Map<defaultMemDbLength_t>>(configuration->get<uint16_t>(ConfigurationKeys::NUMBER_BUCKETS));
        operatorDispatcher_t operatorDispatcher = std::make_shared<OperatorDispatcher>(map, clock, operationLogBuffer, replication, configuration, logger);
        tcpServer_t tcpServer = std::make_shared<TCPServer>(logger, configuration, replication, Authenticator{configuration}, operatorDispatcher);

        return std::make_shared<MemDb>(logger, map, configuration, operatorDispatcher, tcpServer, clock);
    }

private:
    static replication_t createReplicationObject(logger_t logger, configuration_t configuration) {
        if(configuration->getBoolean(ConfigurationKeys::USE_REPLICATION)){
            return ReplicationCreator::setup(configuration, logger);
        }else{
            return std::make_shared<Replication>(logger, configuration);
        }
    }
};