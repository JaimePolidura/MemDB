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
    std::shared_ptr<Request> deserialize(const std::vector<uint8_t>& buffer) {
        std::shared_ptr<Request> request = std::make_shared<Request>();
        request->requestNumber = this->deserializeRequestNumber(buffer);
        request->authentication = this->deserializeAuthenticacion(buffer);
        request->operation = this->deserializeOperation(buffer);

        return request;
    }

private:
    long deserializeRequestNumber(const std::vector<uint8_t> &buffer) {
        long result;

        for (std::size_t i = 0; i < sizeof(long); i++)
            result |= static_cast<long>(buffer[i]) << (8 * i);

        return result;
    }

    std::shared_ptr<AuthenticationBody> deserializeAuthenticacion(const std::vector<uint8_t>& buffer) {
        uint8_t authLength = this->getValueWithoutFlags(buffer, sizeof(long));
        uint8_t * authKey = this->fill(buffer, sizeof(long) + 1, authLength + 1);
        bool flagAuth1 = this->getFlag(buffer, sizeof(long), FLAG1_MASK);
        bool flagAuth2 = this->getFlag(buffer, sizeof(long), FLAG2_MASK);

        return std::make_shared<AuthenticationBody>(std::string((char *) authKey, authLength), flagAuth1, flagAuth2);
    }

    std::shared_ptr<OperationBody> deserializeOperation(const std::vector<uint8_t>& buffer) {
        int authLength = this->getValueWithoutFlags(buffer, sizeof(long));
        int operationBufferInitialPos = authLength + 1;

        int operatorNumber = this->getValueWithoutFlags(buffer, operationBufferInitialPos);
        bool flagOperation1 = this->getFlag(buffer, operationBufferInitialPos, FLAG1_MASK); //Si es true, la longitud de los argumentos ocuparan 2 bytes
        bool flagOperation2 = this->getFlag(buffer, operationBufferInitialPos, FLAG2_MASK);

        if(operationBufferInitialPos == buffer.size() - 1){ //No args
            return std::make_shared<OperationBody>(operatorNumber, flagOperation1, flagOperation2);
        }

        int numerOfArguments = 0;
        int lastIndexChecked = operationBufferInitialPos + 1; //Index first arg
        std::vector<OperatorArgument> arguments;

        while (lastIndexChecked + 1 < buffer.size()) {
            unsigned short argLength = (unsigned short) buffer[lastIndexChecked];
            int argValuePosition = lastIndexChecked + 1;
            uint8_t * argValue = this->fill(buffer, argValuePosition, argValuePosition + argLength);

            lastIndexChecked = argValuePosition + argLength;
            numerOfArguments++;
            arguments.emplace_back(argValue, argLength);
        }

        return std::make_shared<OperationBody>(operatorNumber, flagOperation1, flagOperation2, std::move(arguments), numerOfArguments);
    }

    uint8_t * fill(const std::vector<uint8_t>& buffer, int initialPos, int endPos) {
        int size = endPos - initialPos;
        uint8_t * toFill = new uint8_t[size];

        for(int i = initialPos; i < endPos; i++)
            toFill[i - initialPos] = buffer[i];

        return toFill;
    }

    unsigned short getValueWithoutFlags(const std::vector<uint8_t>& buffer, int pos) {
        return (unsigned short) buffer[pos] >> 2;
    }

    bool getFlag(const std::vector<uint8_t>& buffer, int pos, char flagMask) {
        return (buffer[pos] << 4) & flagMask;
    }
};