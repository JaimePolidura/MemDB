#pragma once

#include "shared.h"
#include "memdbtypes.h"

#include "operators/OperatorNumbers.h"

#include "messages/request/Request.h"

class RequestBuilder {
private:
    std::string _authKey;
    bool _authFlag1{false};
    bool _authFlag2{false};

    memdbRequestNumber_t _requestNumber{0};

    uint8_t _operatorNumber;
    bool _operatorFlag1{false};
    bool _operatorFlag2{false};
    memdbNodeId_t _nodeId{0};
    uint64_t _timestamp{0};
    args_t _args;

public:
    RequestBuilder(): _args(std::make_shared<std::vector<SimpleString<memDbDataLength_t>>>()) {}

    RequestBuilder * authKey(const std::string authKey);
    RequestBuilder * authFlag2(bool value);
    RequestBuilder * auth(const std::string authKey, bool flag1, bool flag2);
    RequestBuilder * requestNumber(memdbRequestNumber_t requestNumber);
    RequestBuilder * operatorNumber(uint8_t operatorNumber);
    RequestBuilder * operatorFlag1(bool flag1);
    RequestBuilder * operatorFlag2(bool flag2);
    RequestBuilder * timestamp(uint64_t timestamp);
    RequestBuilder * selfNode(memdbNodeId_t nodeId);
    RequestBuilder * args(const std::vector<SimpleString<memDbDataLength_t>>& args);
    RequestBuilder * addArg(const SimpleString<memDbDataLength_t>& arg);

    Request build();
    OperationBody buildOperationBody();

    static RequestBuilder builder();
};
