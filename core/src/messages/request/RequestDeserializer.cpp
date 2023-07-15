#include "messages/request/RequestDeserializer.h"

Request RequestDeserializer::deserialize(const std::vector<uint8_t>& buffer) {
    Request request{};
    request.requestNumber = Utils::parseFromBuffer<memdbRequestNumberLength_t>(buffer);
    request.authentication = this->deserializeAuthenticacion(buffer);
    request.operation = this->deserializeOperation(buffer, request.authentication.getTotalLength() + sizeof(memdbRequestNumberLength_t),
                                                   request.authentication.flag1);

    return request;
}

AuthenticationBody RequestDeserializer::deserializeAuthenticacion(const std::vector<uint8_t>& buffer) {
    uint8_t authLength = this->getValueWithoutFlags(buffer, sizeof(memdbRequestNumberLength_t));
    uint8_t * authKey = this->fill(buffer, sizeof(memdbRequestNumberLength_t) + 1, sizeof(memdbRequestNumberLength_t) + authLength + 1);
    bool flagAuth1 = this->getFlag(buffer, sizeof(memdbRequestNumberLength_t), FLAG1_MASK);
    bool flagAuth2 = this->getFlag(buffer, sizeof(memdbRequestNumberLength_t), FLAG2_MASK);

    return AuthenticationBody(std::string((char *) authKey, authLength), flagAuth1, flagAuth2);
}

OperationBody RequestDeserializer::deserializeOperation(const std::vector<uint8_t>& buffer, uint64_t position = 0,
                                   const bool includesNodeId = false) {
    uint8_t operatorNumber = this->getValueWithoutFlags(buffer, position);
    bool flagOperation1 = this->getFlag(buffer, position, FLAG1_MASK); //Si es true, la longitud de los argumentos ocuparan 2 bytes
    bool flagOperation2 = this->getFlag(buffer, position, FLAG2_MASK);

    position++;

    uint64_t timestamp = Utils::parseFromBuffer<uint64_t>(buffer, position);
    position += sizeof(uint64_t);

    memdbNodeId_t nodeId = 1;
    if(includesNodeId) {
        nodeId = Utils::parseFromBuffer<memdbNodeId_t>(buffer, position);
        position += sizeof(uint16_t);
    }

    if(position == buffer.size() - 1){ //No args
        return OperationBody(operatorNumber, flagOperation1, flagOperation2, timestamp, nodeId);
    }

    int numerOfArguments = 0;
    auto arguments = std::make_shared<std::vector<SimpleString<memDbDataLength_t>>>();

    while (position + 1 < buffer.size()) {
        memDbDataLength_t argLength = Utils::parseFromBuffer<memDbDataLength_t>(buffer, position);
        position += sizeof(memDbDataLength_t);

        if(argLength == 0)
            break;

        uint8_t * argValue = this->fill(buffer, position, position + argLength);

        position = position + argLength;
        numerOfArguments++;
        arguments->emplace_back(argValue, argLength);
    }

    return OperationBody(operatorNumber, flagOperation1, flagOperation2, timestamp, nodeId, std::move(arguments));
}

uint8_t * RequestDeserializer::fill(const std::vector<uint8_t>& buffer, int initialPos, int endPos) {
    int size = endPos - initialPos;
    uint8_t * toFill = new uint8_t[size];

    for(int i = initialPos; i < endPos; i++)
        toFill[i - initialPos] = buffer[i];

    return toFill;
}

uint8_t RequestDeserializer::getValueWithoutFlags(const std::vector<uint8_t>& buffer, uint64_t pos) {
    return (uint8_t) buffer[pos] >> 2;
}

bool RequestDeserializer::getFlag(const std::vector<uint8_t>& buffer, uint64_t pos, char flagMask) {
    return (buffer[pos] << 4) & flagMask;
}