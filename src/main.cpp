#include "./connection/TCPServer.h"

#include <memory>

void fillUsersService(const std::shared_ptr<UsersRepository>& usersRepository);

int main() {
    std::shared_ptr<Map> map = std::make_shared<Map>();
    std::shared_ptr<OperatorRegistry> operatorRegistry = std::make_shared<OperatorRegistry>();
    std::shared_ptr<OperatorDispatcher> operatorDispatcher = std::make_shared<OperatorDispatcher>(map, operatorRegistry);
    std::shared_ptr<UsersRepository> usersService = std::make_shared<UsersRepository>() ;
    std::shared_ptr<DynamicThreadPool> threadPool = std::make_shared<DynamicThreadPool>(
            0.9f, 8, 50, 10
    );

    fillUsersService(usersService);

    TCPServer tcpServer{10000, Authenticator{usersService}, operatorDispatcher, threadPool};
    tcpServer.run();

    return 0;
}

void fillUsersService(const std::shared_ptr<UsersRepository>& usersRepository) {
    usersRepository->save(User{"123"});
}