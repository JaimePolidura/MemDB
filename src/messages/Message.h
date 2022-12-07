#pragma once

#include <string>

struct OperatorArgument {
    char * arg;
    int lengthArg;

    OperatorArgument() = default;

    OperatorArgument(char * arg, int lengthArg): arg(arg), lengthArg(lengthArg) {}
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
};

struct AuthenticationBody {
public:
    char * authKey;
    int authKeyLength;
    bool flag1;
    bool flag2;

    AuthenticationBody(char * authKey, int authKeyLength, bool flag1, bool flag2): authKey(authKey), authKeyLength(authKeyLength) , flag1(flag1), flag2(flag2) {}

    AuthenticationBody() = default;
};


struct Message {
    AuthenticationBody authentication;
    OperationBody operation;

    Message() = default;
};

