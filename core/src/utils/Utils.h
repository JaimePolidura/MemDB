#pragma once

#include "shared.h"

class Utils {
public:
    template<typename T>
    static T parse(const uint8_t * input) {
        T parsedValue = 0;
        for(int i = 0; i < sizeof(T); i++) {
            size_t toMove = (sizeof(T) - 1 - i) * 8;
            parsedValue |= static_cast<T>(input[i]) << toMove;
        }

        return parsedValue;
    }

    template<typename T>
    static T parseFromBuffer(const std::vector<uint8_t>& buffer, const uint8_t initialPosition = 0) {
        T result = 0;
        for(int i = sizeof(T) - 1; i >= 0; i--){
            size_t toMove = ((sizeof(T) - 1) - i) * 8;
            result |= buffer[i + initialPosition] << toMove;            
        }

        return result;
    }

    template<typename T>
    static std::vector<uint8_t> appendToBuffer(const T& toParse, std::vector<uint8_t>& vectorOut) {
        for(int i = 0; i < sizeof(T); i++){
            size_t tomove = ((sizeof(T) - 1) - i) * 8;
            vectorOut.push_back(static_cast<uint8_t>((toParse >> tomove)));
        }

        return vectorOut;
    }

    static auto copyFromBuffer(const std::vector<uint8_t>& buffer, const uint8_t initialPosition, const uint8_t endPosition) -> uint8_t * {
        uint8_t * copyDest = new uint8_t[endPosition - initialPosition + 1];
        std::copy(buffer.begin() + initialPosition, buffer.begin() + endPosition + 1, copyDest);

        return copyDest;
    }

    static std::vector<uint8_t> appendToBuffer(const uint8_t * toAppendPtr, const uint8_t totalToAppend, std::vector<uint8_t>& vectorOut) {
        for(auto i = 0; i < totalToAppend; i++) {
            vectorOut.push_back(* (toAppendPtr + i));
        }

        return vectorOut;
    }
};