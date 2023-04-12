#pragma once

#include "shared.h"

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

class DNSUtils {
public:
    static bool isName(const std::string& str) {
        static std::regex IP_REGEX(R"(^(\d{1,3}\.){3}\d{1,3}$)");

        return !std::regex_match(str, IP_REGEX);
    }

    static std::string singleResolve(const std::string& ip, const std::string& port) {
        boost::asio::io_context ioContext{};

        tcp::resolver resolver(ioContext);
        auto const endpointResult = (* resolver.resolve(ip, port)).endpoint();

        return endpointResult.address().to_string();
    }
};