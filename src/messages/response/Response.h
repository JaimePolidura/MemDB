#pragma once

#include <stdint.h>
#include <memory>

struct Response {
public:
    long requestNumber;
    bool isSuccessful;
    uint8_t errorCode;
    uint8_t lengthResponse;
    uint8_t * response;

    Response(bool isSuccessful, uint8_t errorCode, uint8_t lengthResponse, uint8_t * responseCons) :
            isSuccessful(isSuccessful),
            errorCode(errorCode),
            lengthResponse(lengthResponse),
            response(nullptr) {

        if(this->lengthResponse > 0){
            this->response = new uint8_t[lengthResponse];
            std::copy(responseCons, responseCons + lengthResponse, response);
        }
    }

    ~Response() {
        delete[] response;
    }

    static std::shared_ptr<Response> success(uint8_t reponseLength, uint8_t * response) {
        return std::make_shared<Response>(true, 0x00, reponseLength, response);
    }

    static std::shared_ptr<Response> success() {
        return std::make_shared<Response>(true, 0x00, 0, nullptr);
    }

    static std::shared_ptr<Response> error(uint8_t errorCode) {
        return std::make_shared<Response>(false, errorCode, 0, nullptr);
    };
};