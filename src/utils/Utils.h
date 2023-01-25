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
};