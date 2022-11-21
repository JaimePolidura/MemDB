#include <boost/asio.hpp>
#include <string>
#include "TCPServer.h"

TCPServer::TCPServer(const std::string &ip, int port): ip(ip), port(port) {}

void TCPServer::run() {

}