#pragma once

#include <stdint.h>
#include <memory>

#include "utils/strings/SimpleString.h"
#include "memdbtypes.h"
#include "auth/AuthenticationType.h"

struct OperationBody {
    std::shared_ptr<std::vector<SimpleString<defaultMemDbSize_t>>> args;
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

    OperationBody(uint8_t operatorNumber, bool flag1, bool flag2, uint64_t timestamp, uint16_t nodeId, std::shared_ptr<std::vector<SimpleString<defaultMemDbSize_t>>> argsCons):
        flag1(flag1),
        flag2(flag2),
        nodeId(nodeId),
        operatorNumber(operatorNumber),
        timestamp(timestamp),
        args(argsCons) {}
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
};

struct Request {
    AuthenticationBody authentication;
    OperationBody operation;
    uint64_t requestNumber;
    AuthenticationType authenticationType;

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
};

