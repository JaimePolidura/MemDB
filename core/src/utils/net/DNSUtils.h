#pragma once

#include "shared.h"

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

class DNSUtils {
public:
    static std::string singleResolve(const std::string& ip, const std::string& port) {
        boost::asio::io_context ioContext{};

        tcp::resolver resolver(ioContext);
        auto const endpointResult = (* resolver.resolve(ip, port)).endpoint();

        return endpointResult.address().to_string();
    }
};