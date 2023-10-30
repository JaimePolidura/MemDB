#include "RequestBuilder.h"

RequestBuilder RequestBuilder::builder() {
    return RequestBuilder{};
}

RequestBuilder *RequestBuilder::authKey(const std::string authKey) {
    this->_authKey = authKey;
    return this;
}

RequestBuilder *RequestBuilder::authFlag2(bool value) {
    this->_authFlag2 = value;
    return this;
}

RequestBuilder *RequestBuilder::auth(const std::string authKey, bool flag1, bool flag2) {
    this->_authKey = authKey;
    this->_authFlag1 = flag1;
    this->_authFlag2 = flag2;
    return this;
}

RequestBuilder *RequestBuilder::requestNumber(memdbRequestNumber_t requestNumber) {
    this->_requestNumber = requestNumber;
    return this;
}

RequestBuilder *RequestBuilder::operatorNumber(uint8_t operatorNumber) {
    this->_operatorNumber = operatorNumber;
    return this;
}

RequestBuilder *RequestBuilder::operatorFlag1(bool flag1) {
    this->_operatorFlag1 = flag1;
    return this;
}

RequestBuilder *RequestBuilder::operatorFlag2(bool flag2) {
    this->_operatorFlag2 = flag2;
    return this;
}

RequestBuilder *RequestBuilder::timestamp(uint64_t timestamp) {
    this->_timestamp = timestamp;
    return this;
}

RequestBuilder *RequestBuilder::selfNode(memdbNodeId_t nodeId) {
    this->_authFlag1 = true; //Indicates if is node
    this->_nodeId = nodeId;
    return this;
}

RequestBuilder * RequestBuilder::addDoubleArg(uint64_t arg) {
    uint32_t part1 = arg >> 32;
    uint32_t part2 = arg & 0x00000000FFFFFFFF;

    this->_args->push_back(SimpleString<memDbDataLength_t>::fromNumber(part1));
    this->_args->push_back(SimpleString<memDbDataLength_t>::fromNumber(part2));
}

RequestBuilder *RequestBuilder::args(const std::vector<SimpleString<memDbDataLength_t>> &args) {
    this->_args = std::make_shared<std::vector<SimpleString<memDbDataLength_t>>>(args);
    return this;
}

RequestBuilder *RequestBuilder::addArg(const SimpleString<memDbDataLength_t>& arg) {
    this->_args->push_back(arg);
    return this;
}

Request RequestBuilder::build() {
    AuthenticationBody authBody{};
    authBody.authKey = this->_authKey;
    authBody.flag1 = this->_authFlag1;
    authBody.flag2 = this->_authFlag2;

    Request request{};
    request.requestNumber = this->_requestNumber;
    request.operation = this->buildOperationBody();
    request.authentication = authBody;

    return request;
}

OperationBody RequestBuilder::buildOperationBody() {
    OperationBody operationBody{};
    operationBody.operatorNumber = this->_operatorNumber;
    operationBody.flag1 = this->_operatorFlag1;
    operationBody.flag2 = this->_operatorFlag2;
    operationBody.nodeId = this->_nodeId;
    operationBody.timestamp = this->_timestamp;
    operationBody.args = this->_args;

    return operationBody;
}