#pragma once

#include "shared.h"

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

class DNSUtils {
public:
    static bool isName(const std::string& str);

    static std::string singleResolve(const std::string& ip, const std::string& port);
};