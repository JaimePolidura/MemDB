#include "messages/response/ResponseBuilder.h"

ResponseBuilder::ResponseBuilder() {}

ResponseBuilder ResponseBuilder::builder() {
    return ResponseBuilder{}; 
}

ResponseBuilder * ResponseBuilder::timestamp(LamportClock timestamp) {
    this->_timestamp = timestamp;
    return this;
}

ResponseBuilder * ResponseBuilder::timestampCounter(uint64_t timestampCounter) {
    this->_timestamp = LamportClock{0, timestampCounter};
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

ResponseBuilder * ResponseBuilder::isSuccessful(bool isSuccessful, uint8_t errorCodeIfUnsuccessful) {
    this->_isSuccessful = isSuccessful;
    if(!isSuccessful){
        this->_errorCode = errorCodeIfUnsuccessful;
    }

    return this;
}

ResponseBuilder * ResponseBuilder::value(const SimpleString<memDbDataLength_t>& valueParam) {
    this->_responseValue.push_back(valueParam);
    return this;
}

ResponseBuilder * ResponseBuilder::values(const std::vector<SimpleString<memDbDataLength_t>>& values) {
    this->_responseValue = values;
    return this;
}

Response ResponseBuilder::build() {
    return Response{this->_isSuccessful, this->_errorCode, this->_timestamp, this->_requestNumber,
                    this->buildResponseValue()};
}

SimpleString<memDbDataLength_t> ResponseBuilder::buildResponseValue() {
    if(this->_responseValue.empty()){
        return SimpleString<memDbDataLength_t>::fromNumber(0x00);
    } else if (this->_responseValue.size() > 1) {
        return SimpleString<memDbDataLength_t>::fromSimpleStrings(this->_responseValue);
    } else {
        return _responseValue.at(0);
    }
}