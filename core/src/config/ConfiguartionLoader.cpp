#include "config/ConfigurationLoader.h"

std::map<std::string, std::string> argsToConfigMap(int nArgs, char ** args);

configuration_t ConfiguartionLoader::load(int nArgs, char ** args) {
    std::map<std::string, std::string> defaultConfigurationValues = DefaultConfig::get();
    std::map<std::string, std::string> configurationValuesFromArgs = argsToConfigMap(nArgs, args);

    return std::make_shared<Configuration>(Utils::unionMaps(defaultConfigurationValues, configurationValuesFromArgs));
}

std::map<std::string, std::string> argsToConfigMap(int nArgs, char ** args) {
    std::map<std::string, std::string> toReturn{};

    for (int i = 0; i < nArgs; ++i) {
        std::string arg = std::string(*(args + i));
        std::vector<std::string> keyValueArgSplit = StringUtils::split(arg, '=');

        std::string keyArg = keyValueArgSplit[0];
        std::string valueArg = keyValueArgSplit[1];

        toReturn[keyArg] = valueArg;
    }
}