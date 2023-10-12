#pragma once

#include "shared.h"
#include "memdbtypes.h"

#include "operators/OperatorNumbers.h"

#include "messages/request/Request.h"

class RequestBuilder {
private:
    std::string _authKey;
    bool _authFlag1;
    bool _authFlag2;

    memdbRequestNumber_t _requestNumber;

    uint8_t _operatorNumber;
    bool _operatorFlag1;
    bool _operatorFlag2;
    memdbNodeId_t _nodeId;
    uint64_t _timestamp;
    args_t _args;

public:
    RequestBuilder(): _args(std::make_shared<std::vector<SimpleString<memDbDataLength_t>>>()) {}

    RequestBuilder * authKey(const std::string authKey);
    RequestBuilder * authFlag1(bool value);
    RequestBuilder * authFlag2(bool value);
    RequestBuilder * auth(const std::string authKey, bool flag1, bool flag2);
    RequestBuilder * requestNumber(memdbRequestNumber_t requestNumber);
    RequestBuilder * operatorNumber(uint8_t operatorNumber);
    RequestBuilder * operatorFlag1(bool flag1);
    RequestBuilder * operatorFlag2(bool flag2);
    RequestBuilder * timestamp(uint64_t timestamp);
    RequestBuilder * nodeId(memdbNodeId_t nodeId);
    RequestBuilder * args(const std::vector<SimpleString<memDbDataLength_t>>& args);
    RequestBuilder * addArg(const SimpleString<memDbDataLength_t>& arg);

    Request build();
    OperationBody buildOperationBody();

    static RequestBuilder builder();
};
