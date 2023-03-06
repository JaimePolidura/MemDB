#pragma once

#include <stdint.h>
#include <memory>
#include <iostream>

#include "utils/strings/SimpleString.h"
#include "memdbtypes.h"
#include "auth/AuthenticationType.h"

struct OperationBody {
    std::shared_ptr<std::vector<SimpleString<defaultMemDbLength_t>>> args;
    uint64_t timestamp;
    uint16_t nodeId;
    uint8_t operatorNumber; //0 - 127
    bool flag1;
    bool flag2;

    OperationBody() = default;

    OperationBody(uint8_t operatorNumber, bool flag1, bool flag2, uint64_t timestamp, uint16_t nodeId):
        flag1(flag1),
        flag2(flag2),
        nodeId(nodeId),
        operatorNumber(operatorNumber),
        timestamp(timestamp) {}

    OperationBody(uint8_t operatorNumber, bool flag1, bool flag2, uint64_t timestamp, uint16_t nodeId, std::shared_ptr<std::vector<SimpleString<defaultMemDbLength_t>>> argsCons):
        flag1(flag1),
        flag2(flag2),
        nodeId(nodeId),
        operatorNumber(operatorNumber),
        timestamp(timestamp),
        args(argsCons) {}

    bool operator==(const OperationBody& a) const {
        return a.timestamp == this->timestamp &&
            a.nodeId == this->nodeId &&
            a.operatorNumber == this->operatorNumber &&
            a.flag1 == this->flag1 &&
            a.flag2 == this->flag2 &&
            a.args.get() == this->args.get();
    }

    defaultMemDbRequestLength_t getTotalLength(bool includesNodeId) const {
        defaultMemDbRequestLength_t totalLength = 0;
        totalLength += 1; //Op number
        totalLength += 8; //Timestamp
        if(includesNodeId) totalLength += 2;

        for(auto arg = this->args->begin(); arg < this->args->end(); arg++) {
            totalLength += sizeof(defaultMemDbLength_t); //Arg length
            totalLength += arg->size; //Arg length
        }

        return totalLength;
    }
};

struct AuthenticationBody {
public:
    std::string authKey;
    bool flag1;
    bool flag2;

    AuthenticationBody(std::string authKey, bool flag1, bool flag2): authKey(authKey), flag1(flag1), flag2(flag2) {}

    AuthenticationBody() = default;

    AuthenticationBody(AuthenticationBody&& other) noexcept :
            flag1(other.flag1),
            flag2(other.flag2),
            authKey(std::move(other.authKey)){}

    AuthenticationBody& operator=(const AuthenticationBody& other) {
        this->authKey = other.authKey;
        this->flag1 = other.flag1;
        this->flag2 = other.flag2;

        return * this;
    }

    AuthenticationBody& operator=(AuthenticationBody&& other) noexcept {
        this->authKey = std::move(other.authKey);
        this->flag1 = other.flag1;
        this->flag2 = other.flag2;

        return * this;
    }

    defaultMemDbRequestLength_t getTotalLength() const {
        return 1 + authKey.size();
    }
};

struct Request {
    AuthenticationBody authentication;
    OperationBody operation;
    defaultMemDbRequestNumberLength_t requestNumber;
    AuthenticationType authenticationType; //Not in serializetion

    Request(const Request& other) {
        this->authentication = other.authentication;
        this->operation = other.operation;
        this->requestNumber = other.requestNumber;
        this->authenticationType = other.authenticationType;
    }

    Request(Request&& other) noexcept :
        authentication(std::move(other.authentication)),
        operation(std::move(other.operation)),
        requestNumber(other.requestNumber),
        authenticationType(other.authenticationType) {
    };

    Request& operator=(Request&& other) noexcept {
        this->authentication = std::move(other.authentication);
        this->operation = std::move(other.operation);
        this->requestNumber = other.requestNumber;
        this->authenticationType = other.authenticationType;

        return * this;
    }

    Request() = default;

    defaultMemDbRequestLength_t getTotalLength(bool includeNodeId = false) const {
        defaultMemDbRequestLength_t length = 0;
        length += sizeof(defaultMemDbRequestNumberLength_t);
        length += this->authentication.getTotalLength();
        length += this->operation.getTotalLength(includeNodeId);

        return length;
    }
};

