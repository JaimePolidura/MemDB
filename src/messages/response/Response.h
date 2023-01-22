#pragma once

#include <stdint.h>
#include <memory>

#include "utils/strings/SimpleString.h"

struct Response {
public:
    SimpleString responseValue;
    uint64_t requestNumber;
    uint8_t errorCode;
    bool isSuccessful;

    Response(bool isSuccessful, uint8_t errorCode, const SimpleString &response) :
            isSuccessful(isSuccessful),
            responseValue(response),
            errorCode(errorCode)
    {}

    static Response success(const SimpleString &response) {
        return Response(true, 0x00, response);
    }

    static Response success() {
        return Response(true, 0x00, SimpleString::empty());
    }

    static Response error(uint8_t errorCode) {
        return Response(false, errorCode, SimpleString::empty());
    };

    static Response error(uint8_t errorCode, uint64_t requestNumber) {
        Response response = Response(false, errorCode, SimpleString::empty());
        response.requestNumber = requestNumber;

        return response;
    };
};