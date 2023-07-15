#include "messages/request/Request.h"

OperationBody::OperationBody(uint8_t operatorNumber, bool flag1, bool flag2, uint64_t timestamp, memdbNodeId_t nodeId):
    flag1(flag1),
    flag2(flag2),
    nodeId(nodeId),
    operatorNumber(operatorNumber),
    timestamp(timestamp) {}

OperationBody::OperationBody(uint8_t operatorNumber, bool flag1, bool flag2, uint64_t timestamp, memdbNodeId_t nodeId,
        std::shared_ptr<std::vector<SimpleString<memDbDataLength_t>>> argsCons):
    flag1(flag1),
    flag2(flag2),
    nodeId(nodeId),
    operatorNumber(operatorNumber),
    timestamp(timestamp),
    args(argsCons) {}

bool OperationBody::operator==(const OperationBody& a) const {
    return a.timestamp == this->timestamp &&
           a.nodeId == this->nodeId &&
           a.operatorNumber == this->operatorNumber &&
           a.flag1 == this->flag1 &&
           a.flag2 == this->flag2 &&
           a.args.get() == this->args.get();
}

memDbDataLength_t OperationBody::getTotalLength(bool includesNodeId) const {
    memDbDataLength_t totalLength = 0;
    totalLength += 1; //Op number
    totalLength += 8; //Timestamp
    if(includesNodeId) totalLength += 2;

    for(auto arg = this->args->begin(); arg < this->args->end(); arg++) {
        totalLength += sizeof(memDbDataLength_t); //Arg length
        totalLength += arg->size; //Arg length
    }

    return totalLength;
}

arg_t OperationBody::getArg(int position) const {
    return this->args->at(position);
}

void OperationBody::setArg(int position, arg_t newArg) {
    if(position > this->args->size()){
        this->args->push_back(newArg);
    }else{
        auto argValue = this->args->begin() + position;
        *argValue = newArg;
    }
}

args_t OperationBody::createOperationBodyArg() {
    return std::make_shared<std::vector<SimpleString<memDbDataLength_t>>>();
}

AuthenticationBody::AuthenticationBody(std::string authKey, bool flag1, bool flag2): authKey(authKey), flag1(flag1), flag2(flag2) {}

AuthenticationBody::AuthenticationBody(AuthenticationBody&& other) noexcept :
    flag1(other.flag1),
    flag2(other.flag2),
    authKey(std::move(other.authKey)){}

AuthenticationBody& AuthenticationBody::operator=(const AuthenticationBody& other) {
    this->authKey = other.authKey;
    this->flag1 = other.flag1;
    this->flag2 = other.flag2;

    return * this;
}

AuthenticationBody& AuthenticationBody::operator=(AuthenticationBody&& other) noexcept {
    this->authKey = std::move(other.authKey);
    this->flag1 = other.flag1;
    this->flag2 = other.flag2;

    return * this;
}

memDbDataLength_t AuthenticationBody::getTotalLength() const {
    return 1 + authKey.size();
}

Request::Request(const Request& other) {
    this->authentication = other.authentication;
    this->operation = other.operation;
    this->requestNumber = other.requestNumber;
    this->authenticationType = other.authenticationType;
}

Request::Request(Request&& other) noexcept :
    authentication(std::move(other.authentication)),
    operation(std::move(other.operation)),
    requestNumber(other.requestNumber),
    authenticationType(other.authenticationType) {
};

Request & Request::operator=(Request&& other) noexcept {
    this->authentication = std::move(other.authentication);
    this->operation = std::move(other.operation);
    this->requestNumber = other.requestNumber;
    this->authenticationType = other.authenticationType;

    return * this;
}

memDbDataLength_t Request::getTotalLength() const {
    memDbDataLength_t length = 0;
    length += sizeof(memdbRequestNumberLength_t);
    length += this->authentication.getTotalLength();
    length += this->operation.getTotalLength(authentication.flag1);

    return length;
}