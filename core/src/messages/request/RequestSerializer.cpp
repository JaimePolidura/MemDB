#include "messages/request/RequestSerializer.h"

std::vector<uint8_t> RequestSerializer::serialize(const Request& request) {
    std::vector<uint8_t> bytes{};

    Utils::appendToBuffer(request.requestNumber, bytes);

    bytes.push_back(request.authentication.authKey.size() << 2 | request.authentication.flag1 << 1 | request.authentication.flag2);
    Utils::appendToBuffer((uint8_t *) request.authentication.authKey.data(), request.authentication.authKey.size(), bytes);
    bytes.push_back(request.operation.operatorNumber << 2 | request.operation.flag1 << 1 | request.operation.flag2);
    Utils::appendToBuffer(request.operation.timestamp, bytes);
    if(request.authentication.flag1) //Includes selfNode
        Utils::appendToBuffer(request.operation.nodeId, bytes);

    for(auto i = request.operation.args->begin(); i < request.operation.args->end(); i++) {
        SimpleString<memDbDataLength_t> arg = * i;

        Utils::appendToBuffer(arg.size, bytes);
        Utils::appendToBuffer(arg.data(), arg.size, bytes);
    }

    return bytes;
}