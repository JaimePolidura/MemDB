#pragma once

#include <memory>

#include "utils/strings/SimpleString.h"
#include "memdbtypes.h"

struct Response {
public:
    SimpleString<defaultMemDbSize_t> responseValue;
    uint64_t requestNumber;
    uint64_t timestamp;
    uint8_t errorCode;
    bool isSuccessful;

    Response(bool isSuccessful, uint8_t errorCode, uint64_t timestamp, const SimpleString<defaultMemDbSize_t> &response) :
            isSuccessful(isSuccessful),
            responseValue(response),
            timestamp(timestamp),
            errorCode(errorCode)
    {}
    
    static Response success(const SimpleString<defaultMemDbSize_t> &response, uint64_t timestamp = 0) {
        return Response(true, 0x00, timestamp, response);
    }

    static Response success(uint64_t timestamp = 0) {
        return Response(true, 0x00, timestamp, SimpleString<defaultMemDbSize_t>::empty());
    }

    static Response error(uint8_t errorCode, uint64_t timestamp = 0) {
        return Response(false, errorCode, timestamp, SimpleString<defaultMemDbSize_t>::empty());
    };

    static Response error(uint8_t errorCode, uint64_t requestNumber, uint64_t timestamp = 0) {
        Response response = Response(false, errorCode, timestamp, SimpleString<defaultMemDbSize_t>::empty());
        response.requestNumber = requestNumber;

        return response;
    };
};