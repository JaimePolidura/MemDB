#pragma once

#include "shared.h"
#include "utils/strings/StringUtils.h"

class Configuration {
private:
    const std::map<std::string, std::string> actualConfigurationValues;
    const std::map<std::string, std::string> defaultConfigurationValues;

public:
    Configuration() = default;

    Configuration(const std::map<std::string, std::string>& actualConfigurationValues,
                  const std::map<std::string, std::string>& defaultConfigurationValues):
        actualConfigurationValues(std::move(actualConfigurationValues)),
        defaultConfigurationValues(std::move(defaultConfigurationValues)) {}

    virtual std::string get(const std::string& key) {
        return this->getOrDefault(key);
    }

    virtual bool getBoolean(const std::string& key) {
        return this->getOrDefault(key) == "true";
    }

    template<typename T>
    T get(const std::string& key) {
        static_assert(!std::is_same<T, bool>::value, "Use getBoolean() for boolean values!");

        return static_cast<T>(std::stoul(this->getOrDefault(key)));
    }

private:
    std::string getOrDefault(const std::string& key) {
        bool containedInActual = this->actualConfigurationValues.contains(key);

        return containedInActual ?
            this->actualConfigurationValues.at(key) :
            this->defaultConfigurationValues.at(key);
    }
};

using configuration_t = std::shared_ptr<Configuration>;