#pragma once

#include <stdint.h>

struct Response {
public:
    bool isSuccessful;
    uint8_t errorCode;
    uint8_t lengthResponse;
    uint8_t * response;

    Response(bool isSuccessful, uint8_t errorCode, uint8_t lengthResponse, uint8_t * responseCons) :
            isSuccessful(isSuccessful),
            errorCode(errorCode),
            lengthResponse(lengthResponse),
            response(new uint8_t[lengthResponse]) {
        std::copy(responseCons, responseCons + lengthResponse, response);
    }

    ~Response() {
        delete[] response;
    }

    static Response success(uint8_t reponseLength, uint8_t * response) {
        return Response{true, 0x00, reponseLength, response};
    }

    static Response success() {
        return Response{true, 0x00, 0, nullptr};
    }

    static Response error(uint8_t errorCode) {
        return Response{false, errorCode, 0, nullptr};
    };
};