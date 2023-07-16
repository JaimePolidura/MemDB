#include "utils/net/HttpClient.h"

HttpClient::HttpClient(logger_t logger): logger(logger) {}

HttpResponse HttpClient::get(const std::string& address,
                 const std::string& endpoint,
                 const std::string& authToken) {
    tcp::socket socket(ioContext);
    connect(socket, address);

    http::request<http::string_body> req{http::verb::get, endpoint, 11};
    req.set(http::field::host, address);
    req.set(http::field::user_agent, "Boost");
    req.set(http::field::content_type, "application/json");
    if(!authToken.empty()){
        req.set(http::field::authorization, "Bearer " + authToken);
    }

    req.prepare_payload();

    logger->debugInfo("Sending GET request to {0} {1}", endpoint, address);

    http::write(socket, req);

    boost::beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;
    http::read(socket, buffer, res);

    auto responseString = boost::beast::buffers_to_string(res.body().data());

    socket.close();

    return HttpResponse{
            .body = nlohmann::json::parse(responseString),
            .code = res.result_int()
    };
}

HttpResponse HttpClient::post(const std::string& address,
                  const std::string& endpoint,
                  const std::map<std::string, std::string>& body,
                  const std::string& authToken) {
    tcp::socket socket(ioContext);
    connect(socket, address);

    http::request<http::string_body> req{http::verb::post, endpoint, 11};
    req.set(http::field::host, address);
    req.set(http::field::user_agent, "Boost");
    req.set(http::field::content_type, "application/json");
    if(!authToken.empty()){
        req.set(http::field::authorization, "Bearer " + authToken);
    }

    nlohmann::json json(body);
    req.body() = json.dump();

    req.prepare_payload();

    logger->debugInfo("Sending POST request to {0} {1}", endpoint, address);

    http::write(socket, req);

    boost::beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;
    http::read(socket, buffer, res);

    auto responseString = boost::beast::buffers_to_string(res.body().data());

    socket.close();

    return HttpResponse{
            .body = nlohmann::json::parse(responseString),
            .code = res.result_int()
    };
}

void HttpClient::connect(tcp::socket& socket, const std::string& address) {
    auto urlValues = StringUtils::split(address, ':');
    std::string ipUrl = urlValues[0];
    std::string portUrl = urlValues[1];

    if(DNSUtils::isName(ipUrl)){
        ipUrl = DNSUtils::singleResolve(ipUrl, portUrl);
    }

    bool succcess = Utils::retryUntil(10, std::chrono::seconds(5), [&socket, ipUrl, portUrl](){
        tcp::endpoint endpoint(boost::asio::ip::address::from_string(ipUrl), std::atoi(portUrl.data()));
        socket.connect(endpoint);
    });

    if(!succcess)
        throw std::runtime_error("Cannot connect to " + address );
}


bool HttpResponse::isSuccessful() const {
    return code >= 200 && code < 300;
}

bool HttpResponse::isForbidden() const {
    return this->code == 403;
}