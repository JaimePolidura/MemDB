#include "messages/response/Response.h"

Response::Response(bool isSuccessful, uint8_t errorCode, LamportClock timestamp, memdbRequestNumber_t reqNumber, const SimpleString<memDbDataLength_t> &response) :
        isSuccessful(isSuccessful),
        responseValue(response),
        requestNumber(reqNumber),
        timestamp(timestamp),
        errorCode(errorCode)
{}

memDbDataLength_t Response::getTotalLength() const {
    return sizeof(memdbRequestNumber_t) + sizeof(uint64_t) + sizeof(uint16_t) + 1 + (responseValue.size > 0 ? (sizeof(memDbDataLength_t) + responseValue.size) : sizeof(memDbDataLength_t));
}

bool Response::hasErrorCode(uint8_t errorCodeToCheck) {
    return (this->errorCode >> 1 & errorCodeToCheck) == errorCodeToCheck;
}

SimpleString<memDbDataLength_t> Response::getResponseValueAtOffset(uint32_t initOffset, std::size_t size) const {
    return SimpleString<memDbDataLength_t>::fromPointer(this->responseValue.data() + initOffset, size);
}

Response Response::success(const SimpleString<memDbDataLength_t> &response, LamportClock timestamp) {
    return Response(true, 0x00, timestamp, 0, response);
}

Response Response::success(LamportClock timestamp) {
    return Response(true, 0x00, timestamp, 0, SimpleString<memDbDataLength_t>::empty());
}

Response Response::error(uint8_t errorCode) {
    return Response(false, errorCode, LamportClock{0, 0}, 0, SimpleString<memDbDataLength_t>::empty());
}

Response Response::error(uint8_t errorCode, memdbRequestNumber_t requestNumber, LamportClock timestamp) {
    Response response = Response(false, errorCode, timestamp, 0, SimpleString<memDbDataLength_t>::empty());
    response.requestNumber = requestNumber;

    return response;
}