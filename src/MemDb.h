#pragma once

#include "./server/TCPServer.h"
#include "config/Configuration.h"
#include "config/keys/ConfiguartionKeys.h"
#include <memory>

class MemDb {
private:
    std::shared_ptr<Map> map;
    std::shared_ptr<Configuration> configuration;
    std::shared_ptr<UsersRepository> usersRepository;
    std::shared_ptr<TCPServer> tcpServer;

public:
    MemDb(std::shared_ptr<Map> map, std::shared_ptr<Configuration> configuration, std::shared_ptr<UsersRepository> usersRepository,
          std::shared_ptr<TCPServer> tcpServer)
            : map(map), configuration(configuration), usersRepository(usersRepository), tcpServer(tcpServer) {}

    void run() {
        this->tcpServer->run();
    }
};