#pragma once

#include "./keys/ConfiguartionKeys.h"

#include <string>
#include <map>

class DefaultConfiguration {
public:
    static const std::map<std::string, std::string>& get() {
        static const std::map<std::string, std::string> defaultConfigurationValues = {
                { ConfiguartionKeys::AUTH_KEY, "123" },
        };

        return defaultConfigurationValues;
    }
};