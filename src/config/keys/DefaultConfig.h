#pragma once

#include <string>
#include <vector>
#include <map>

#include "config/keys/ConfiguartionKeys.h"

class DefaultConfig {
public:
    static std::map<std::string, std::string> get() {
        return {
                { ConfiguartionKeys::AUTH_KEY, "123", },
                { ConfiguartionKeys::SERVER_MIN_THREADS, "20"},
                { ConfiguartionKeys::SERVER_MAX_THREADS, "100"},
                { ConfiguartionKeys::PORT, "10000"},
                { ConfiguartionKeys::BUCKETS, "64" }
        };
    }
};