#include "config/ConfigurationLoader.h"

configuration_t ConfiguartionLoader::load() {
    std::map<std::string, std::string> defaultConfiguartionValues = DefaultConfig::get();

    return std::make_shared<Configuration>(defaultConfiguartionValues);
}