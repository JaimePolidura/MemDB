#pragma once

#include "server/TCPServer.h"
#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "persistence/OperationLogDiskLoader.h"

#include <memory>

class MemDb {
private:
    std::shared_ptr<OperatorDispatcher> operatorDispatcher;
    std::shared_ptr<UsersRepository> usersRepository;
    std::shared_ptr<Configuration> configuration;
    std::shared_ptr<TCPServer> tcpServer;
    std::shared_ptr<Map> dbMap;

public:
    MemDb(std::shared_ptr<Map> map, std::shared_ptr<Configuration> configuration, std::shared_ptr<UsersRepository> usersRepository,
          std::shared_ptr<OperatorDispatcher> operatorDispatcher, std::shared_ptr<TCPServer> tcpServer)
            : dbMap(map), configuration(configuration), usersRepository(usersRepository), tcpServer(tcpServer), operatorDispatcher(operatorDispatcher)
          {}

    void run() {
        this->restoreDataFromOplog();
        this->tcpServer->run();
    }

private:
    void restoreDataFromOplog() {
        if(!this->configuration->getBoolean(ConfigurationKeys::USE_PERSISTENCE))
            return;

        OperationLogDiskLoader loader(this->operatorDispatcher);
        loader.loadIntoMapDb(this->dbMap);
    }
};