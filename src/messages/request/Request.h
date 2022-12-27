#pragma once

#include <stdint.h>

//TODO Añadir shared pointers a los datos que estan en el heap
struct OperatorArgument {
    uint8_t * arg;
    int lengthArg;

    OperatorArgument() = default;

    OperatorArgument(uint8_t * arg, int lengthArg): arg(arg), lengthArg(lengthArg) {}
};

struct OperationBody {
    std::vector<OperatorArgument> args;
    int operatorNumber; //0 - 127
    int numberArgs;
    bool flag1;
    bool flag2;

    OperationBody() = default;

    OperationBody(int operatorNumber, bool flag1, bool flag2): flag1(flag1), flag2(flag2), operatorNumber(operatorNumber) {}

    OperationBody(int operatorNumber, bool flag1, bool flag2, std::vector<OperatorArgument> args, int numberArgs):
        flag1(flag1),
        flag2(flag2),
        operatorNumber(operatorNumber),
        args(args),
        numberArgs(numberArgs) {}
};

struct AuthenticationBody {
public:
    std::string authKey;
    bool flag1;
    bool flag2;

    AuthenticationBody(std::string authKey, bool flag1, bool flag2): authKey(authKey), flag1(flag1), flag2(flag2) {}

    AuthenticationBody() = default;
};

struct Request {
    std::shared_ptr<AuthenticationBody> authentication;
    std::shared_ptr<OperationBody> operation;
    uint64_t requestNumber;

    Request() = default;
};

