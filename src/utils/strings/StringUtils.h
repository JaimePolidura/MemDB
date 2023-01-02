#pragma once

#include <sstream>
#include <vector>
#include <string>

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
};