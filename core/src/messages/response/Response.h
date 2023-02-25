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
    
    static Response<ResponseValueLengthType> success(const SimpleString<ResponseValueLengthType> &response, uint64_t timestamp = 0) {
        return Response<ResponseValueLengthType>(true, 0x00, timestamp, response);
    }

    static Response<ResponseValueLengthType> success(uint64_t timestamp = 0) {
        return Response<ResponseValueLengthType>(true, 0x00, timestamp, SimpleString<ResponseValueLengthType>::empty());
    }

    static Response<ResponseValueLengthType> error(uint8_t errorCode, uint64_t timestamp = 0) {
        return Response<ResponseValueLengthType>(false, errorCode, timestamp, SimpleString<ResponseValueLengthType>::empty());
    };

    static Response<ResponseValueLengthType> error(uint8_t errorCode, uint64_t requestNumber, uint64_t timestamp = 0) {
        Response response = Response<ResponseValueLengthType>(false, errorCode, timestamp, SimpleString<ResponseValueLengthType>::empty());
        response.requestNumber = requestNumber;

        return response;
    };
};