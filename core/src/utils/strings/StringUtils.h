#pragma once

#include "shared.h"

class StringUtils {
public:
    static std::vector<std::string> split(const std::string& stringToSplit, char delimiter);

    template<typename T>
    static std::string toString(const T& toStringValue) {
        std::ostringstream ss;
        ss << toStringValue;
        return ss.str();
    }
};