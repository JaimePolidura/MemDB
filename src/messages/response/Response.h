#pragma once

#include <stdint.h>

struct Response {
public:
    bool isSuccessful;
    uint8_t errorCode;
    uint8_t lengthResponse;
    uint8_t * response;

    Response(uint8_t lengthResponse, uint8_t * response): isSuccessful(true), lengthResponse(lengthResponse), response(response), errorCode(0x00) {}

    Response(uint8_t errorCode, uint8_t lengthResponse, uint8_t * response): isSuccessful(false), lengthResponse(lengthResponse), response(response), errorCode(errorCode) {}

    Response(uint8_t errorCode): isSuccessful(false), lengthResponse(0), response(nullptr), errorCode(errorCode) {}

    Response(): isSuccessful(true), lengthResponse(0), response(nullptr), errorCode(0) {}

    ~Response() {
        delete[] response;
    }

    static Response error(uint8_t errorCode) {
        return Response{errorCode};
    };
};