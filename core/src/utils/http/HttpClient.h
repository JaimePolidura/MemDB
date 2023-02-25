#pragma once

#include <nlohmann/json.hpp>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <string>
#include <map>

#include "utils/strings/StringUtils.h"

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
public:
    static HttpResponse post(const std::string& address,
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
    static void connect(tcp::socket& socket, io_context& ioContext, const std::string& address, const bool usingDns) {
        auto urlValues = StringUtils::split(address, ':');
        auto ipUrl = urlValues[0];
        auto portUrl = urlValues[1];

        if(usingDns){
            tcp::resolver resolver(ioContext);
            auto const results = resolver.resolve(ipUrl, portUrl);

            boost::asio::connect(socket, results);
        }else{
            tcp::endpoint endpoint(boost::asio::ip::address::from_string(ipUrl), std::atoi(portUrl.data()));
            socket.connect(endpoint);
        }
    }
};