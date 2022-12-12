#include "./connection/TCPServer.h"

#include <memory>

int main() {
    std::shared_ptr<UsersRepository> usersRepository = std::make_shared<UsersRepository>();
    std::shared_ptr<DynamicThreadPool> threadPool = std::make_shared<DynamicThreadPool>(
            0.9f, 8, 50, 10
    );

    TCPServer tcpServer{10000, Authenticator{usersRepository}, threadPool};
    tcpServer.run();

    return 0;
}