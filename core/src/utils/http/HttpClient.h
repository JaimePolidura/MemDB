#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <map>
#include <cpr/cpr.h>

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

        cpr::Header headers = {{"Content-Type", "application/json"}};

        if(authToken != "")
            headers.insert({{"Authorization", "Bearer " + authToken}});

        cpr::Response response = cpr::Post(cpr::Url(url), body, headers, cpr::Timeout(30 * 1000));

        return HttpResponse{
                .body = nlohmann::json::parse(response.text),
                .code = response.status_code
        };
    }
};