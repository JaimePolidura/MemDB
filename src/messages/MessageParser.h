#pragma once

#include "Message.h"
#include <memory>
#include <vector>
#include <bitset>

class MessageParser {
private:
    static const uint8_t FLAG1_MASK = 0x20; //1000 0000
    static const uint8_t FLAG2_MASK = 0x10; //0100 0000

public:
    std::shared_ptr<Message> parse(const std::vector<uint8_t>& buffer) {
        std::shared_ptr<Message> message = std::make_shared<Message>();
        message->authentication = this->parseAuthenticacion(buffer);
        message->operation = this->parseOperation(buffer);

        return message;
    }

private:
    const AuthenticationBody * parseAuthenticacion(const std::vector<uint8_t>& buffer) {
        unsigned short authLength = this->getValueWithoutFlags(buffer, 0);
        char * authKey = this->fill(buffer, 1, authLength);
        bool flagAuth1 = this->getFlag(buffer, 0, FLAG1_MASK);
        bool flagAuth2 = this->getFlag(buffer, 0, FLAG2_MASK);

        return new AuthenticationBody{authKey, authLength, flagAuth1, flagAuth2};
    }

    const OperationBody * parseOperation(const std::vector<uint8_t>& buffer) {
        int authLength = this->getValueWithoutFlags(buffer, 0);
        int operationBufferInitialPos = authLength + 1;

        int operatorNumber = this->getValueWithoutFlags(buffer, operationBufferInitialPos);
        bool flagOperation1 = this->getFlag(buffer, operationBufferInitialPos, FLAG1_MASK); //Si es true, la longitud de los argumentos ocuparan 2 bytes
        bool flagOperation2 = this->getFlag(buffer, operationBufferInitialPos, FLAG2_MASK);

        if(operationBufferInitialPos == buffer.size() - 1){ //No args
            return new OperationBody{operatorNumber, flagOperation1, flagOperation2};
        }

        int numerOfArguments = 0;
        int lastIndexChecked = operationBufferInitialPos + 1; //Index first arg
        std::vector<OperatorArgument> arguments;

        while (lastIndexChecked + 1 < buffer.size()) {
            unsigned short argLength = this->getValue(buffer, lastIndexChecked, flagOperation1);
            int argValuePosition = lastIndexChecked + (flagOperation1 ? 2 : 1);
            char * argValue = this->fill(buffer, argValuePosition, argValuePosition + argLength);

            lastIndexChecked = argValuePosition + argLength;
            numerOfArguments++;
            arguments.emplace_back(argValue, argLength);
        }

        return new OperationBody{operatorNumber, flagOperation1, flagOperation2, arguments.data(), numerOfArguments};
    }

    char * fill(const std::vector<uint8_t>& buffer, int initialPos, int endPos) {
        int size = endPos - initialPos;
        char * toFill = new char[size];

        for(int i = initialPos; i <= endPos; i++)
            toFill[i - initialPos] = buffer[i];

        return toFill;
    }

    unsigned short getValue(const std::vector<uint8_t>& vector, int pos, bool occupiesTwoSlots = false) {
        return (unsigned short) ( occupiesTwoSlots && vector[pos + 1] != 0x00 ? //Lenght of arg occupies two bytes and second byte is not null (0x00)
                ((unsigned short) vector[pos]) << 8 | vector[pos + 1] :
                vector[pos]);
    }

    unsigned short getValueWithoutFlags(const std::vector<uint8_t>& buffer, int pos) {
        return (unsigned short) buffer[pos] >> 2;
    }

    bool getFlag(const std::vector<uint8_t>& buffer, int pos, char flagMask) {
        return (buffer[pos] << 4) & flagMask;
    }
};