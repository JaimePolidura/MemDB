#pragma once

#include <stdint.h>
#include <memory>

struct Response {
public:
    uint8_t * response;
    uint64_t requestNumber;
    uint8_t errorCode;
    uint8_t lengthResponse;
    bool isSuccessful;

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

    Response(Response&& other) noexcept :
        response(other.response), requestNumber(other.requestNumber), errorCode(other.errorCode),
        lengthResponse(other.lengthResponse), isSuccessful(other.isSuccessful)
    {
        other.response = nullptr;
    }

    ~Response() {
        if(this->response)
            delete[] response;
    }

    static Response success(uint8_t reponseLength, uint8_t * response) {
        return Response(true, 0x00, reponseLength, response);
    }

    static Response success() {
        return Response(true, 0x00, 0, nullptr);
    }

    static Response error(uint8_t errorCode) {
        return Response(false, errorCode, 0, nullptr);
    };
};