#pragma once

#include "shared.h"

#include "utils/strings/StringUtils.h"
#include "utils/net/DNSUtils.h"
#include "utils/Utils.h"

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

struct HttpResponse {
    nlohmann::json body;
    uint32_t code;

    bool isSuccessful() {
        return code >= 200 && code < 300;
    }

    bool isForbidden() {
        return this->code == 403;
    }
};

class HttpClient {
private:
    boost::asio::io_context ioContext;

public:
    HttpResponse get(const std::string& address,
                             const std::string& endpoint,
                             const std::string& authToken = "") {
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

    HttpResponse post(const std::string& address,
                             const std::string& endpoint,
                             const std::map<std::string, std::string>& body = {},
                             const std::string& authToken = "") {
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

private:
    void connect(tcp::socket& socket, const std::string& address) {
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
};