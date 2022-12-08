#pragma once

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>

using namespace boost::asio;

class TCPConnection : public std::enable_shared_from_this<TCPConnection> {
private:
    ip::tcp::socket socket;
    char data[2];

public:
    explicit TCPConnection(ip::tcp::socket socket) : socket{std::move(socket)} {}

    void read() {
        std::shared_ptr<TCPConnection> self {shared_from_this()};

        this->socket.async_read_some(boost::asio::buffer(data, 2), [this, self](boost::system::error_code ec, std::size_t length){
            printf("Read bytes %llu\n", length);

            if(!ec){
                printf("Message: %s\n", this->data);
            }
        });
    }
};