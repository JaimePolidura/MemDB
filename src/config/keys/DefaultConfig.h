#pragma once

#include <string>
#include <vector>
#include <map>

#include "config/keys/ConfigurationKeys.h"

class DefaultConfig {
public:
    static std::map<std::string, std::string> get() {
        return {
                {ConfigurationKeys::AUTH_KEY,                "123", },
                {ConfigurationKeys::SERVER_MIN_THREADS,      "20"},
                {ConfigurationKeys::SERVER_MAX_THREADS,      "100"},
                {ConfigurationKeys::PORT,                    "10000"},
                {ConfigurationKeys::NUMBER_BUCKETS,          "64" },
                {ConfigurationKeys::PERSISTANCE_WRITE_EVERY, "50" },
                {ConfigurationKeys::USE_REPLICATION,         "false"}
        };
    }
};