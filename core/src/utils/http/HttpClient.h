#pragma once

#include "shared.h"

#include "utils/strings/StringUtils.h"

#include "logging/Logger.h"

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
    logger_t logger;

public:
    HttpClient(logger_t logger): logger(logger) {}

    HttpResponse get(const std::string& address,
                             const std::string& endpoint,
                             const std::string& authToken = "",
                             const bool usingDns = false) {
        boost::asio::io_context ioc;
        tcp::socket socket(ioc);
        connect(socket, ioc, address, usingDns);

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
                             const std::string& authToken = "",
                             const bool usingDns = false) {
        boost::asio::io_context ioc;
        tcp::socket socket(ioc);
        connect(socket, ioc, address, usingDns);

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
    void connect(tcp::socket& socket, boost::asio::io_context& ioContext, const std::string& address, const bool usingDns) {
        auto urlValues = StringUtils::split(address, ':');
        auto ipUrl = urlValues[0];
        auto portUrl = urlValues[1];

        int remainingAttemptsToConnect = 10;
        while(!tryConnect(socket, ioContext, ipUrl, portUrl, usingDns)){
            remainingAttemptsToConnect--;

            this->logger->warn("Retrying to send http request to {0}", address.data());

            if(remainingAttemptsToConnect == 0){
                throw std::runtime_error("Cannot connect to " + address );
            }

            std::this_thread::sleep_for(std::chrono::microseconds(250));
        }
    }

    bool tryConnect(tcp::socket& socket, boost::asio::io_context& ioContext, const std::string& ip, const std::string& port, const bool usingDns) {
        try{
            if (usingDns) {
                tcp::resolver resolver(ioContext);
                auto const results = resolver.resolve(ip, port);

                boost::asio::connect(socket, results);
            } else {
                tcp::endpoint endpoint(boost::asio::ip::address::from_string(ip), std::atoi(port.data()));
                socket.connect(endpoint);
            }

            return true;
        }catch(const std::exception& e) {
            return false;
        }
    }
};