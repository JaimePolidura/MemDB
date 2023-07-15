#include "config/Configuration.h"

Configuration::Configuration(const std::map<std::string, std::string>& defaultConfigurationValues):
        defaultConfigurationValues(std::move(defaultConfigurationValues)) {}

std::string Configuration::get(const std::string& key) {
    return this->getOrDefault(key);
}

bool Configuration::getBoolean(const std::string& key) {
    return this->getOrDefault(key) == "true";
}

std::string Configuration::getOrDefault(const std::string& key) {
    char * fromEnvVariable = std::getenv(key.data());
    bool containedInActual = fromEnvVariable != nullptr;

    return containedInActual ?
           std::string(fromEnvVariable) :
           this->getDefault(key);
}

std::string Configuration::getDefault(const std::string& key) {
    if(!this->defaultConfigurationValues.contains(key)) {
        throw std::runtime_error("Env variable not declared: " + key);
    }

    return this->defaultConfigurationValues.at(key);
}