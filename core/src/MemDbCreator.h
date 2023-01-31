#pragma once

#include <memory>

#include "MemDb.h"
#include "config/ConfigurationLoader.h"
#include "config/keys/ConfigurationKeys.h"
#include "replication/ReplicationNode.h"

class MemDbCreator {
public:
    static std::shared_ptr<MemDb> create() {
        std::shared_ptr<Configuration> configuration = ConfiguartionLoader::load();

        std::shared_ptr<LamportClock> clock = std::make_shared<LamportClock>(1);
        std::shared_ptr<ReplicationNode> replicationNode = std::make_shared<ReplicationNode>(1, clock); //TODO Temporal

        std::shared_ptr<OperationLogBuffer> operationLogBuffer = std::make_shared<OperationLogBuffer>(configuration);

        std::shared_ptr<Map> map = std::make_shared<Map>(configuration->get<uint16_t>(ConfigurationKeys::NUMBER_BUCKETS));

        std::shared_ptr<UsersRepository> usersRepository = getUsersRepository(configuration);

        std::shared_ptr<OperatorDispatcher> operatorDispatcher = std::make_shared<OperatorDispatcher>(map, clock, operationLogBuffer);

        std::shared_ptr<TCPServer> tcpServer = std::make_shared<TCPServer>(configuration, Authenticator{usersRepository}, operatorDispatcher);

        return std::make_shared<MemDb>(map, configuration, usersRepository, operatorDispatcher, tcpServer, replicationNode);
    }

private:
    static std::shared_ptr<UsersRepository> getUsersRepository(std::shared_ptr<Configuration> configuration) {
        std::string authKey = configuration->get(ConfigurationKeys::AUTH_KEY);
        std::shared_ptr<UsersRepository> usersRepository = std::make_shared<UsersRepository>();
        usersRepository->save({authKey});

        return usersRepository;
    }
};