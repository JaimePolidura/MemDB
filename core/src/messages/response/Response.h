#pragma once

#include "shared.h"

#include "utils/strings/SimpleString.h"
#include "memdbtypes.h"

struct Response {
public:
    SimpleString<memDbDataLength_t> responseValue;
    memdbRequestNumberLength_t requestNumber;
    uint64_t timestamp;
    uint8_t errorCode;
    bool isSuccessful;

    Response(bool isSuccessful, uint8_t errorCode, uint64_t timestamp, memdbRequestNumberLength_t reqNumber, const SimpleString<memDbDataLength_t> &response) :
            isSuccessful(isSuccessful),
            responseValue(response),
            requestNumber(reqNumber),
            timestamp(timestamp),
            errorCode(errorCode)
    {}

    memDbDataLength_t getTotalLength() const {
        return sizeof(memdbRequestNumberLength_t) +
            sizeof(timestamp) +
            1  + //Error code
            responseValue.size > 0 ? (sizeof(memDbDataLength_t) + sizeof(responseValue.size)) : 0; //Responde body
    }

    static Response success(const SimpleString<memDbDataLength_t> &response, uint64_t timestamp = 0) {
        return Response(true, 0x00, timestamp, 0, response);
    }

    static Response success(uint64_t timestamp = 0) {
        return Response(true, 0x00, timestamp, 0, SimpleString<memDbDataLength_t>::empty());
    }

    static Response error(uint8_t errorCode) {
        return Response(false, errorCode, 0, 0, SimpleString<memDbDataLength_t>::empty());
    };

    static Response error(uint8_t errorCode, memdbRequestNumberLength_t requestNumber, uint64_t timestamp = 0) {
        Response response = Response(false, errorCode, timestamp, 0, SimpleString<memDbDataLength_t>::empty());
        response.requestNumber = requestNumber;

        return response;
    };
};