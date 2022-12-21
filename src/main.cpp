#include "./connection/TCPServer.h"

#include <memory>

int main() {
    std::shared_ptr<Map> map = std::make_shared<Map>();
    std::shared_ptr<OperatorDispatcher> operatorDispatcher = std::make_shared<OperatorDispatcher>(map);
    std::shared_ptr<UsersRepository> usersService = std::make_shared<UsersRepository>() ;
    std::shared_ptr<DynamicThreadPool> threadPool = std::make_shared<DynamicThreadPool>(
            0.9f, 8, 50, 10
    );

    TCPServer tcpServer{10000, Authenticator{usersService}, operatorDispatcher, threadPool};
    tcpServer.run();

    return 0;
}