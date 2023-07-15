#pragma once

#include "config/keys/ConfigurationKeys.h"
#include "utils/strings/StringUtils.h"
#include "shared.h"

class Configuration {
private:
    const std::map<std::string, std::string> defaultConfigurationValues;

public:
    Configuration() = default;

    Configuration(const std::map<std::string, std::string>& defaultConfigurationValues);

    virtual std::string get(const std::string& key);

    virtual bool getBoolean(const std::string& key);

    template<typename T>
    T get(const std::string& key) {
        static_assert(!std::is_same<T, bool>::value, "Use getBoolean() for boolean values!");

        return static_cast<T>(std::stoul(this->getOrDefault(key)));
    }

private:
    std::string getOrDefault(const std::string& key);

    std::string getDefault(const std::string& key);
};

using configuration_t = std::shared_ptr<Configuration>;