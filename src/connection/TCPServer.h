#pragma once

#include <string>

class TCPServer {
private:
    std::string ip;
    int port;

public:
    TCPServer(const std::string &ip, int port);

    void run();
};