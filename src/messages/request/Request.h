#pragma once

#include <stdint.h>

struct OperatorArgument {
    uint8_t * arg;
    int lengthArg;

    OperatorArgument() = default;

    OperatorArgument(uint8_t * arg, int lengthArg): arg(arg), lengthArg(lengthArg) {}

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

    ~ OperatorArgument() {
        if(this->arg)
            delete[] arg;
    }
};

struct OperationBody {
    std::vector<OperatorArgument> args;
    int operatorNumber; //0 - 127
    int numberArgs;
    bool flag1;
    bool flag2;

    OperationBody() = default;

    OperationBody(int operatorNumber, bool flag1, bool flag2): flag1(flag1), flag2(flag2), operatorNumber(operatorNumber) {}

    OperationBody(int operatorNumber, bool flag1, bool flag2, std::vector<OperatorArgument> argsCons, int numberArgs):
        flag1(flag1),
        flag2(flag2),
        operatorNumber(operatorNumber),
        args(std::move(argsCons)),
        numberArgs(numberArgs) {}

    OperationBody(OperationBody&& other) noexcept :
        args(std::move(other.args)),
        operatorNumber(other.operatorNumber),
        numberArgs(other.operatorNumber),
        flag1(other.flag1),
        flag2(other.flag2) {}

    OperationBody& operator=(const OperationBody& other) noexcept {
        this->args = other.args;
        this->operatorNumber = other.operatorNumber;
        this->numberArgs = other.numberArgs;
        this->flag1 = other.flag1;
        this->flag2 = other.flag2;

        return * this;
    }

    OperationBody& operator=(OperationBody&& other) noexcept {
        this->args = std::move(other.args);
        this->operatorNumber = other.operatorNumber;
        this->numberArgs = other.numberArgs;
        this->flag1 = other.flag1;
        this->flag2 = other.flag2;

        return * this;
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

