#include "messages/response/ResponseBuilder.h"

ResponseBuilder::ResponseBuilder(): _responseValue(SimpleString<memDbDataLength_t>::fromNumber(0)) {}

ResponseBuilder ResponseBuilder::builder() {
    return ResponseBuilder{}; 
}

ResponseBuilder * ResponseBuilder::timestamp(uint64_t timestamp) {
    this->_timestamp = timestamp;
    return this;
}

ResponseBuilder * ResponseBuilder::error(uint8_t errorCode) {
    this->_errorCode = errorCode;
    this->_isSuccessful = false;
    return this;
}

ResponseBuilder * ResponseBuilder::success() {
    this->_isSuccessful = true;
    return this;
}

ResponseBuilder * ResponseBuilder::requestNumber(memdbRequestNumber_t requestNumber) {
    this->_requestNumber = requestNumber;
    return this;
}

ResponseBuilder * ResponseBuilder::value(const SimpleString<memDbDataLength_t>& value) {
    this->_responseValue = value;
    return this;
}

Response ResponseBuilder::build() {
    return Response{this->_isSuccessful, this->_errorCode, this->_timestamp, this->_requestNumber, this->_responseValue};
}