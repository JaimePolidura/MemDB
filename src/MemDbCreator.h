#pragma once

#include <memory>

#include "MemDb.h"
#include "config/ConfigurationLoader.h"
#include "config/keys/ConfiguartionKeys.h"

class MemDbCreator {
public:
    static std::shared_ptr<MemDb> create() {
        std::shared_ptr<Configuration> configuration = ConfiguartionLoader::load();
        std::shared_ptr<Map> map = std::make_shared<Map>(configuration->get<uint16_t>(ConfiguartionKeys::NUMBER_BUCKETS));
        std::shared_ptr<UsersRepository> usersRepository = getUsersRepository(configuration);
        std::shared_ptr<OperatorDispatcher> operatorDispatcher = std::make_shared<OperatorDispatcher>(map);
        std::shared_ptr<TCPServer> tcpServer = std::make_shared<TCPServer>(configuration, Authenticator{usersRepository}, operatorDispatcher);
        
        return std::make_shared<MemDb>(map, configuration, usersRepository, tcpServer);
    }

private:
    static std::shared_ptr<UsersRepository> getUsersRepository(std::shared_ptr<Configuration> configuration) {
        std::string authKey = configuration->get(ConfiguartionKeys::AUTH_KEY);
        std::shared_ptr<UsersRepository> usersRepository = std::make_shared<UsersRepository>();
        usersRepository->save({authKey});

        return usersRepository;
    }
};