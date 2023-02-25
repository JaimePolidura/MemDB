#pragma once

#include <stdint.h>
#include <memory>

#include "utils/strings/SimpleString.h"

template<typename ResponseValueLengthType = uint8_t>
struct Response {
    static_assert(std::is_unsigned_v<ResponseValueLengthType>, "ResponseValueLengthType should be unsigned");

public:
    SimpleString<ResponseValueLengthType> responseValue;
    uint64_t requestNumber;
    uint64_t timestamp;
    uint8_t errorCode;
    bool isSuccessful;

    Response(bool isSuccessful, uint8_t errorCode, uint64_t timestamp, const SimpleString<ResponseValueLengthType> &response) :
            isSuccessful(isSuccessful),
            responseValue(response),
            timestamp(timestamp),
            errorCode(errorCode)
    {}

    size_t getResponseValueLength() {
        return sizeof(ResponseValueLengthType);
    }
    
    template<typename ResponseValueLengthType_success1 = uint8_t>
    static Response success(const SimpleString<ResponseValueLengthType_success1> &response, uint64_t timestamp = 0) {
        return Response(true, 0x00, timestamp, response);
    }

    template<typename ResponseValueLengthType_success2 = uint8_t>
    static Response success(uint64_t timestamp = 0) {
        return Response(true, 0x00, timestamp, SimpleString<ResponseValueLengthType_success2>::empty());
    }

    template<typename ResponseValueLengthType_error1 = uint8_t>
    static Response error(uint8_t errorCode, uint64_t timestamp = 0) {
        return Response(false, errorCode, timestamp, SimpleString<ResponseValueLengthType_error1>::empty());
    };
    
    template<typename ResponseValueLengthType_error2 = uint8_t>
    static Response error(uint8_t errorCode, uint64_t requestNumber, uint64_t timestamp = 0) {
        Response response = Response(false, errorCode, timestamp, SimpleString<ResponseValueLengthType_error2>::empty());
        response.requestNumber = requestNumber;

        return response;
    };
};