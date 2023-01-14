#pragma once

#include "Request.h"

#include <memory>
#include <vector>
#include <bitset>

class RequestDeserializer {
private:
    static const uint8_t FLAG1_MASK = 0x20; //1000 0000
    static const uint8_t FLAG2_MASK = 0x10; //0100 0000

public:
    Request deserialize(const std::vector<uint8_t>& buffer) {
        uint8_t authLength = this->getValueWithoutFlags(buffer, sizeof(uint64_t));

        Request request{};
        request.requestNumber = this->deserializeRequestNumber(buffer);
        request.authentication = this->deserializeAuthenticacion(buffer);
        request.operation = this->deserializeOperation(buffer, authLength + sizeof(uint64_t) + 1);

        return request;
    }

    uint64_t deserializeRequestNumber(const std::vector<uint8_t> &buffer) {
        uint64_t result = 0;

        for (std::size_t i = 0; i < sizeof(uint64_t); i++)
            result |= static_cast<uint64_t>(buffer[i]) << (sizeof(uint64_t) * i);

        return result;
    }

    AuthenticationBody deserializeAuthenticacion(const std::vector<uint8_t>& buffer) {
        uint8_t authLength = this->getValueWithoutFlags(buffer, sizeof(uint64_t));
        uint8_t * authKey = this->fill(buffer, sizeof(uint64_t) + 1, sizeof(uint64_t) + authLength + 1);
        bool flagAuth1 = this->getFlag(buffer, sizeof(uint64_t), FLAG1_MASK);
        bool flagAuth2 = this->getFlag(buffer, sizeof(uint64_t), FLAG2_MASK);

        return AuthenticationBody(std::string((char *) authKey, authLength), flagAuth1, flagAuth2);
    }

    OperationBody deserializeOperation(const std::vector<uint8_t>& buffer, const uint8_t initialOffset = 0) {
        uint8_t operatorNumber = this->getValueWithoutFlags(buffer, initialOffset);
        bool flagOperation1 = this->getFlag(buffer, initialOffset, FLAG1_MASK); //Si es true, la longitud de los argumentos ocuparan 2 bytes
        bool flagOperation2 = this->getFlag(buffer, initialOffset, FLAG2_MASK);

        if(initialOffset == buffer.size() - 1){ //No args
            return OperationBody(operatorNumber, flagOperation1, flagOperation2);
        }

        int numerOfArguments = 0;
        int lastIndexChecked = initialOffset + 1; //Index first arg
        std::shared_ptr<std::vector<OperatorArgument>> arguments = std::make_shared<std::vector<OperatorArgument>>();

        while (lastIndexChecked + 1 < buffer.size()) {
            unsigned short argLength = (unsigned short) buffer[lastIndexChecked];
            if(argLength == 0)
                break;

            int argValuePosition = lastIndexChecked + 1;
            uint8_t * argValue = this->fill(buffer, argValuePosition, argValuePosition + argLength);

            lastIndexChecked = argValuePosition + argLength;
            numerOfArguments++;
            arguments->emplace_back(argValue, argLength);
        }

        return OperationBody(operatorNumber, flagOperation1, flagOperation2, std::move(arguments));
    }

private:
    uint8_t * fill(const std::vector<uint8_t>& buffer, int initialPos, int endPos) {
        int size = endPos - initialPos;
        uint8_t * toFill = new uint8_t[size];

        for(int i = initialPos; i < endPos; i++)
            toFill[i - initialPos] = buffer[i];

        return toFill;
    }

    uint8_t getValueWithoutFlags(const std::vector<uint8_t>& buffer, int pos) {
        return (uint8_t) buffer[pos] >> 2;
    }

    bool getFlag(const std::vector<uint8_t>& buffer, int pos, char flagMask) {
        return (buffer[pos] << 4) & flagMask;
    }
};