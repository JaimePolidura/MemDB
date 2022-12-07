#include "./connection/TCPServer.h"

int main() {
    UsersRepository usersRepository{};

    TCPServer tcpServer{10000, Authenticator{usersRepository}};
    tcpServer.run();

    return 0;
}