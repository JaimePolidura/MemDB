#pragma once

#include <vector>
#include <cstdint>

class Utils {
public:
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
    static std::vector<uint8_t> parseToBuffer(const T& toParse, std::vector<uint8_t>& vectorOut, const uint8_t positionToWrite = 0) {
        for(int i = 0; i < sizeof(T); i++){
            size_t tomove = ((sizeof(T) - 1) - i) * sizeof(uint8_t) * 8;
            vectorOut.insert(vectorOut.begin() + (i + positionToWrite), static_cast<uint8_t>((toParse >> tomove) & 0xFF));
        }

        return vectorOut;
    }
};