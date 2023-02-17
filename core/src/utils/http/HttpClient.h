#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <map>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/JSON/Parser.h>

using namespace Poco::Net;
using namespace Poco::JSON;

struct HttpResponse {
    nlohmann::json body;
    int code;

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

        HTTPClientSession session(url);
        HTTPRequest request(HTTPRequest::HTTP_POST, url, HTTPMessage::HTTP_1_1);
        request.setContentType("application/json");

        if(authToken != "")
            request.set("Authorization", "Bearer " + authToken);

        nlohmann::json jsonRequest;
        for (const auto& pair : body) {
            jsonRequest[pair.first] = pair.second;
        }

        std::ostream& requestStrem = session.sendRequest(request);
        requestStrem << jsonRequest.dump();

        Poco::Net::HTTPResponse response;
        std::istream& responseStream = session.receiveResponse(response);

        std::stringstream output_stream;
        output_stream << responseStream.rdbuf();
        std::string output_string = output_stream.str();

        return HttpResponse{
                .body = nlohmann::json::parse(output_string),
                .code = response.getStatus()
        };
    }
};