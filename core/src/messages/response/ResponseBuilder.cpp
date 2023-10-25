#include "messages/response/ResponseBuilder.h"

ResponseBuilder::ResponseBuilder() {}

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

ResponseBuilder * ResponseBuilder::value(const SimpleString<memDbDataLength_t>& valueParam) {
    this->_responseValue.push_back(valueParam);
    return this;
}

ResponseBuilder * ResponseBuilder::values(const std::vector<SimpleString<memDbDataLength_t>>& values) {
    this->_responseValue.insert(this->_responseValue.begin(), values.begin(), values.end());
    return this;
}

Response ResponseBuilder::build() {
    return Response{this->_isSuccessful, this->_errorCode, this->_timestamp, this->_requestNumber,
                    _responseValue.size() > 1 ?
                        SimpleString<memDbDataLength_t >::fromSimpleStrings(this->_responseValue) :
                        _responseValue.at(0)};
}