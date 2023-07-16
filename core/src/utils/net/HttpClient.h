#pragma once

#include "shared.h"

#include "utils/strings/StringUtils.h"
#include "utils/net/DNSUtils.h"
#include "utils/Utils.h"
#include "logging/Logger.h"

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

struct HttpResponse {
    nlohmann::json body;
    uint32_t code;

    bool isSuccessful() const;

    bool isForbidden() const;
};

class HttpClient {
private:
    boost::asio::io_context ioContext;
    logger_t logger;

public:
    HttpClient(logger_t logger);

    HttpResponse get(const std::string& address,
                             const std::string& endpoint,
                             const std::string& authToken = "");

    HttpResponse post(const std::string& address,
                             const std::string& endpoint,
                             const std::map<std::string, std::string>& body = {},
                             const std::string& authToken = "");

private:
    void connect(tcp::socket& socket, const std::string& address);
};