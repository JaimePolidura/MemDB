#pragma once

#include "Message.h"
#include <memory>
#include <vector>

class MessageParser {
private:
    static const char FLAG1_MASK = 0x80; //1000 0000
    static const char FLAG2_MASK = 0x40; //0100 0000

public:
    std::shared_ptr<Message> parse(const std::vector<char>& buffer) {
        std::shared_ptr<Message> message = std::make_shared<Message>();
        message->authentication = this->parseAuthenticacion(buffer);
        message->operation = this->parseOperation(buffer);

        return message;
    }

    const AuthenticationBody& parseAuthenticacion(const std::vector<char>& buffer) {
        unsigned short authLength = this->getValueWithoutFlags(buffer, 0);
        char * authKey = this->fill(buffer, 1, authLength);
        bool flagAuth1 = this->getFlag(buffer, 0, FLAG1_MASK);
        bool flagAuth2 = this->getFlag(buffer, 0, FLAG2_MASK);

        return AuthenticationBody{authKey, authLength, flagAuth1, flagAuth2};
    }

    const OperationBody& parseOperation(const std::vector<char>& buffer) {
        int authLength = this->getValueWithoutFlags(buffer, 0);
        int operationBufferInitialPos = authLength + 1;

        int operatorNumber = this->getValueWithoutFlags(buffer, operationBufferInitialPos);
        bool flagOperation1 = this->getFlag(buffer, operationBufferInitialPos, FLAG1_MASK); //Si es true, la longitud de los argumentos ocuparan 2 bytes
        bool flagOperation2 = this->getFlag(buffer, operationBufferInitialPos, FLAG2_MASK);

        if(operationBufferInitialPos == buffer.size() - 1){ //No args
            return OperationBody{operatorNumber, flagOperation1, flagOperation2};
        }

        int numerOfArguments = 0;
        int lastIndexChecked = operationBufferInitialPos;
        std::vector<OperatorArgument> arguments;

        while (lastIndexChecked + 1 < buffer.size()) {
            unsigned short argLength = this->getValue(buffer, lastIndexChecked + 1, flagOperation1);
            int argValuePosition = operationBufferInitialPos + (flagOperation1 ? 2 : 1);
            char * argValue = this->fill(buffer, argValuePosition, argValuePosition + argLength);

            lastIndexChecked = argValuePosition;
            numerOfArguments++;
            arguments.emplace_back(OperatorArgument(argValue, argLength));
        }

        return OperationBody{operatorNumber, flagOperation1, flagOperation2, arguments.data(), numerOfArguments};
    }

    char * fill(const std::vector<char>& buffer, int initialPos, int endPos) {
        int size = endPos - initialPos;
        char * toFill = new char[size];

        for(int i = initialPos; i <= endPos; i++)
            toFill[i - initialPos] = buffer[i];

        return toFill;
    }

    unsigned short getValue(const std::vector<char>& vector, int pos, bool occupiesTwoSlots = false) {
        return (unsigned short) ( occupiesTwoSlots ?
                vector[pos] << 8 | vector[pos + 1] :
                vector[pos]);
    }

    unsigned short getValueWithoutFlags(const std::vector<char>& buffer, int pos) {
        return (unsigned short) buffer[pos] >> 2;
    }

    bool getFlag(const std::vector<char>& buffer, int pos, char flagMask) {
        return (buffer[pos] << 6) & flagMask;
    }
};