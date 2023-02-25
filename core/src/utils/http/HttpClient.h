#pragma once

#include <nlohmann/json.hpp>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <string>
#include <map>

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
    static HttpResponse post(const std::string& url,
                             const std::map<std::string, std::string>& body = {},
                             const std::string& authToken = "") {
        boost::asio::io_context ioc;
        tcp::resolver resolver(ioc);
        tcp::socket socket(ioc);

        auto const results = resolver.resolve(url, "http");
        boost::asio::connect(socket, results.begin(), results.end());

        http::request<http::string_body> req{http::verb::post, "post", 11};
        req.set(http::field::host, url);
        req.set(http::field::user_agent, "Boost");
        req.set(http::field::content_type, "application/json");
        if(!authToken.empty()) req.set(http::field::authorization, "Bearer " + authToken);

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
};