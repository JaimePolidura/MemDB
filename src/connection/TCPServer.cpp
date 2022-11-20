#include <boost/asio.hpp>
#include <iostream>
#include <string>

using namespace boost::asio;

class TCPServer {
private:
    ip::tcp::acceptor acceptator;
    io_context ioContext;

public:
    TCPServer(std::string ip, int port): ioContext(), acceptator(ioContext, ip::tcp::endpoint(ip::tcp::v4(), port)) {}

    void run() {
        while (true) {
            auto connection = this->acceptator.accept();

        }
    }
};