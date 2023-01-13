#pragma once

#include <stdint.h>
#include <memory>

struct OperatorArgument {
    uint8_t * arg;
    uint8_t lengthArg;

    OperatorArgument() = default;

    OperatorArgument(uint8_t * arg, uint8_t lengthArg): arg(arg), lengthArg(lengthArg) {}

    OperatorArgument(const OperatorArgument& other) {
        this->arg = other.arg;
        this->lengthArg = other.lengthArg;
    }

    OperatorArgument& operator=(const OperatorArgument& other) noexcept {
        this->arg = other.arg;
        this->lengthArg = other.lengthArg;

        return * this;
    }

    OperatorArgument(OperatorArgument&& other) noexcept :
        arg(other.arg),
        lengthArg(other.lengthArg) {
        other.arg = nullptr;
    }

    OperatorArgument& operator=(OperatorArgument&& other) noexcept {
        this->arg = other.arg;
        this->lengthArg = other.lengthArg;
        other.arg = nullptr;

        return * this;
    }

    ~OperatorArgument() {
        //Do nothing, in this way arg won't get deleted, arg is stored in the hashmap as the raw pointer, so the
        // refcount won't get updated
    }
};

struct OperationBody {
    std::shared_ptr<std::vector<OperatorArgument>> args;
    uint8_t operatorNumber; //0 - 127
    bool flag1;
    bool flag2;

    OperationBody() = default;

    OperationBody(uint8_t operatorNumber, bool flag1, bool flag2): flag1(flag1), flag2(flag2), operatorNumber(operatorNumber) {}

    OperationBody(uint8_t operatorNumber, bool flag1, bool flag2, std::shared_ptr<std::vector<OperatorArgument>> argsCons, uint8_t numberArgs):
        flag1(flag1),
        flag2(flag2),
        operatorNumber(operatorNumber),
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

    Request(const Request& other) {
        this->authentication = other.authentication;
        this->operation = other.operation;
        this->requestNumber = other.requestNumber;
    }

    Request(Request&& other) noexcept :
        authentication(std::move(other.authentication)),
        operation(std::move(other.operation)),
        requestNumber(other.requestNumber) {
    };

    Request& operator=(Request&& other) noexcept {
        this->authentication = std::move(other.authentication);
        this->operation = std::move(other.operation);
        this->requestNumber = other.requestNumber;

        return * this;
    }

    Request() = default;
};

