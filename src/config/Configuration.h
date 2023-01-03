#pragma once

#include <map>
#include <string>

class Configuration {
private:
    const std::map<std::string, std::string> configurationValues;

public:
    Configuration(const std::map<std::string, std::string>& configurationValues): configurationValues(std::move(configurationValues)) {}

    std::string get(const std::string& key) const {
        return this->configurationValues.at(key);
    }

    template<typename T>
    T get(const std::string& key) const {
        return static_cast<T>(std::stoul(this->configurationValues.at(key)));
    }
};