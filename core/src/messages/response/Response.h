#pragma once

#include <stdint.h>
#include <memory>

#include "utils/strings/SimpleString.h"

struct Response {
public:
    SimpleString responseValue;
    uint64_t requestNumber;
    uint64_t timestamp;
    uint8_t errorCode;
    bool isSuccessful;

    Response(bool isSuccessful, uint8_t errorCode, uint64_t timestamp, const SimpleString &response) :
            isSuccessful(isSuccessful),
            responseValue(response),
            timestamp(timestamp),
            errorCode(errorCode)
    {}

    static Response success(const SimpleString &response, uint64_t timestamp = 0) {
        return Response(true, 0x00, timestamp, response);
    }

    static Response success(uint64_t timestamp = 0) {
        return Response(true, 0x00, timestamp, SimpleString::empty());
    }

    static Response error(uint8_t errorCode, uint64_t timestamp = 0) {
        return Response(false, errorCode, timestamp, SimpleString::empty());
    };

    static Response error(uint8_t errorCode, uint64_t requestNumber, uint64_t timestamp = 0) {
        Response response = Response(false, errorCode, timestamp, SimpleString::empty());
        response.requestNumber = requestNumber;

        return response;
    };
};