#pragma once

#include <vector>
#include <cstdint>

class Utils {
public:
    template<typename T>
    static T parseFromBuffer(const std::vector<uint8_t>& buffer, const uint8_t initialPosition = 0) {
        T result;
        for(int i = 0; i < sizeof(T); i++)
            result |= static_cast<T>(buffer.at(i + initialPosition));

        return result;
    }

    template<typename T>
    static std::vector<uint8_t> parseToBuffer(const T& toParse, std::vector<uint8_t>& vectorOut, const uint8_t positionToWrite = 0) {
        for(int i = 0; i < sizeof(T); i++)
            vectorOut[i + positionToWrite] = static_cast<uint8_t>(toParse >> (i * sizeof(uint8_t) & 0xFF));

        return vectorOut;
    }
};