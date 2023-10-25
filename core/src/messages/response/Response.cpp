#include "messages/response/Response.h"

Response::Response(bool isSuccessful, uint8_t errorCode, uint64_t timestamp, memdbRequestNumber_t reqNumber, const SimpleString<memDbDataLength_t> &response) :
        isSuccessful(isSuccessful),
        responseValue(response),
        requestNumber(reqNumber),
        timestamp(timestamp),
        errorCode(errorCode)
{}

memDbDataLength_t Response::getTotalLength() const {
    return sizeof(memdbRequestNumber_t) + sizeof(uint64_t) + 1 + (responseValue.size > 0 ? (sizeof(memDbDataLength_t) + responseValue.size) : sizeof(memDbDataLength_t));
}

SimpleString<memDbDataLength_t> Response::getResponseValueAtOffset(uint32_t initOffset, std::size_t size) const {
    return SimpleString<memDbDataLength_t>::fromPointer(this->responseValue.data() + initOffset, size);
}

Response Response::success(const SimpleString<memDbDataLength_t> &response, uint64_t timestamp) {
    return Response(true, 0x00, timestamp, 0, response);
}

Response Response::success(uint64_t timestamp) {
    return Response(true, 0x00, timestamp, 0, SimpleString<memDbDataLength_t>::empty());
}

Response Response::error(uint8_t errorCode) {
    return Response(false, errorCode, 0, 0, SimpleString<memDbDataLength_t>::empty());
}

Response Response::error(uint8_t errorCode, memdbRequestNumber_t requestNumber, uint64_t timestamp) {
    Response response = Response(false, errorCode, timestamp, 0, SimpleString<memDbDataLength_t>::empty());
    response.requestNumber = requestNumber;

    return response;
}