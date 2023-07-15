#include "utils/net/DNSUtils.h"

bool DNSUtils::isName(const std::string& str) {
    static std::regex IP_REGEX(R"(^(\d{1,3}\.){3}\d{1,3}$)");

    return !std::regex_match(str, IP_REGEX);
}

std::string DNSUtils::singleResolve(const std::string& ip, const std::string& port) {
    boost::asio::io_context ioContext{};

    tcp::resolver resolver(ioContext);
    auto const endpointResult = (* resolver.resolve(ip, port)).endpoint();

    return endpointResult.address().to_string();
}
