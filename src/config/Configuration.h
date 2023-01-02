#pragma once

#include <map>
#include <string>

class Configuration {
private:
    std::map<std::string, std::string> configurationValues;

public:
    Configuration(const std::map<std::string, std::string>& configurationValues): configurationValues(std::move(configurationValues)) {}

    std::string get(const std::string& key) {
        return this->configurationValues.at(key);
    }

    template<typename T>
    T get(const std::string& key) {
        return (T) this->configurationValues.at(key);
    }
};