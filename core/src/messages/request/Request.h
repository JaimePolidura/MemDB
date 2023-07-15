#pragma once

#include "shared.h"

#include "utils/strings/SimpleString.h"
#include "memdbtypes.h"
#include "auth/AuthenticationType.h"

using arg_t = SimpleString<memDbDataLength_t>;
using args_t = std::shared_ptr<std::vector<SimpleString<memDbDataLength_t>>>;

struct OperationBody {
    args_t args;
    uint64_t timestamp;
    memdbNodeId_t nodeId; //2 bytes
    uint8_t operatorNumber; //0 - 127
    bool flag1;
    bool flag2;

    OperationBody() = default;

    OperationBody(uint8_t operatorNumber, bool flag1, bool flag2, uint64_t timestamp, memdbNodeId_t nodeId);

    OperationBody(uint8_t operatorNumber, bool flag1, bool flag2, uint64_t timestamp, memdbNodeId_t nodeId,
                  std::shared_ptr<std::vector<SimpleString<memDbDataLength_t>>> argsCons);

    bool operator==(const OperationBody& a) const;

    memDbDataLength_t getTotalLength(bool includesNodeId) const;

    arg_t getArg(int position) const;

    void setArg(int position, arg_t newArg);

    static args_t createOperationBodyArg();
};

struct AuthenticationBody {
public:
    std::string authKey;
    bool flag1; //Includes nodeid in request payload
    bool flag2;

    AuthenticationBody(std::string authKey, bool flag1, bool flag2);

    AuthenticationBody() = default;

    AuthenticationBody(AuthenticationBody&& other) noexcept;

    AuthenticationBody& operator=(const AuthenticationBody& other);

    AuthenticationBody& operator=(AuthenticationBody&& other) noexcept;

    memDbDataLength_t getTotalLength() const;
};

struct Request {
    AuthenticationBody authentication;
    OperationBody operation;
    memdbRequestNumberLength_t requestNumber;
    AuthenticationType authenticationType; //Not in serializetion

    Request(const Request& other);

    Request(Request&& other) noexcept;

    Request& operator=(Request&& other) noexcept;

    Request() = default;

    memDbDataLength_t getTotalLength() const;
};

