#include "./connection/TCPServer.h"

int main() {
    TCPServer tcpServer{"127.0.0.1", 10000};
    tcpServer.run();

    return 0;
}