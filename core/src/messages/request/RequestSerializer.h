#pragma once

#include "Request.h"
#include "utils/Utils.h"

#include <vector>

class RequestSerializer {
public:
    std::vector<uint8_t> serialize(const Request& request, const bool includeNodeIdTimestamp = false) {
        std::vector<uint8_t> bytes{};

        Utils::appendToBuffer(request.requestNumber, bytes);
        bytes.push_back(request.authentication.authKey.size() << 2 | request.authentication.flag1 << 1 | request.authentication.flag2);
        Utils::appendToBuffer((uint8_t *) request.authentication.authKey.data(), request.authentication.authKey.size(), bytes);
        bytes.push_back(request.operation.operatorNumber << 2 | request.operation.flag1 << 1 | request.operation.flag2);
        Utils::appendToBuffer(request.operation.timestamp, bytes);
        if(includeNodeIdTimestamp)
            Utils::appendToBuffer(request.operation.nodeId, bytes);

        for(auto i = request.args->begin(); i < request.args->end(); i++) {
            SimpleString<defaultMemDbSize_t> arg = * i;

            Utils::appendToBuffer(arg.size, bytes);
            Utils::appendToBuffer(arg.data(), arg.size, bytes);
        }

        return bytes;
    }
};