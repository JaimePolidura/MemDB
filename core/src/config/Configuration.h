#pragma once

#include <map>
#include <string>

class Configuration {
private:
    const std::map<std::string, std::string> actualConfigurationValues;
    const std::map<std::string, std::string> defaultConfigurationValues;

public:
    Configuration(const std::map<std::string, std::string>& actualConfigurationValues,
                  const std::map<std::string, std::string>& defaultConfigurationValues):
        actualConfigurationValues(std::move(actualConfigurationValues)),
        defaultConfigurationValues(std::move(defaultConfigurationValues)) {}

    std::string get(const std::string& key) const {
        return this->getOrDefault(key);
    }

    bool getBoolean(const std::string& key) const {
        return this->getOrDefault(key) == "true";
    }

    template<typename T>
    T get(const std::string& key) const {
        static_assert(!std::is_same<T, bool>::value, "Use getBoolean() for boolean values!");

        return static_cast<T>(std::stoul(this->getOrDefault(key)));
    }

private:
    std::string getOrDefault(const std::string& key) const {
        bool containedInActual = this->actualConfigurationValues.contains(key);

        return containedInActual ?
            this->actualConfigurationValues.at(key) :
            this->defaultConfigurationValues.at(key);
    }
};