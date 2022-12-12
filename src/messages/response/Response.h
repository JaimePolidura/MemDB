#pragma once

#include <stdint.h>

struct Response {
    bool isSuccessful;
    uint8_t errorType;
    uint8_t lengthResponse;
    uint8_t * response;

    Response(uint8_t lengthResponse, uint8_t * response): isSuccessful(true), lengthResponse(lengthResponse), response(response), errorType(0x00) {}

    Response(uint8_t errorType, uint8_t lengthResponse, uint8_t * response): isSuccessful(false), lengthResponse(lengthResponse), response(response), errorType(errorType) {}

    Response(uint8_t errorType): isSuccessful(false), lengthResponse(0), response(nullptr), errorType(errorType) {}

    Response(): isSuccessful(true), lengthResponse(0), response(nullptr), errorType(0) {}

    ~Response() {
        delete[] response;
    }
};