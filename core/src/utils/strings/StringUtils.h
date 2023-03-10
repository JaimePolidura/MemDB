#pragma once

#include "shared.h"

class StringUtils {
public:
    static std::vector<std::string> split(const std::string& stringToSplit, char delimiter) {
        std::vector<std::string> tokens;
        std::istringstream stream(stringToSplit);
        std::string token;

        while (std::getline(stream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    template<typename T>
    static std::string toString(const T& toStringValue) {
        std::ostringstream ss;
        ss << toStringValue;
        return ss.str();
    }
};