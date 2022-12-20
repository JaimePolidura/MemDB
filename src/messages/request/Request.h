#pragma once

#include <stdint.h>

struct OperatorArgument {
    uint8_t * arg;
    int lengthArg;

    OperatorArgument() = default;

    OperatorArgument(uint8_t * arg, int lengthArg): arg(arg), lengthArg(lengthArg) {}
};

struct OperationBody {
    int operatorNumber; //0 - 127
    bool flag1;
    bool flag2;
    OperatorArgument * args;
    int numberArgs;

    OperationBody() = default;

    OperationBody(int operatorNumber, bool flag1, bool flag2): flag1(flag1), flag2(flag2), operatorNumber(operatorNumber) {}

    OperationBody(int operatorNumber, bool flag1, bool flag2, OperatorArgument * args, int numberArgs): flag1(flag1), flag2(flag2), operatorNumber(operatorNumber),
                                                                                                        args(args), numberArgs(numberArgs) {}
    ~OperationBody(){
        delete[] this->args;
    }
};

struct AuthenticationBody {
public:
    std::string authKey;
    bool flag1;
    bool flag2;

    AuthenticationBody(std::string authKey, bool flag1, bool flag2): authKey(authKey), flag1(flag1), flag2(flag2) {}

    AuthenticationBody() = default;

    ~AuthenticationBody() {
        delete this->authKey.data();
    }
};

struct Request {
    const AuthenticationBody * authentication;
    const OperationBody * operation;

    Request() = default;

    ~Request() {
        delete this->authentication;
        delete this->operation;
    }
};

