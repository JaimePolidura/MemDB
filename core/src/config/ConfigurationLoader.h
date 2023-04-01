#pragma once

#include "config/Configuration.h"
#include "config/keys/DefaultConfig.h"
#include "utils/files/FileUtils.h"
#include "utils/strings/StringUtils.h"

#include "shared.h"

class ConfiguartionLoader {
public:
    static configuration_t load() {
        std::map<std::string, std::string> defaultConfiguartionValues = DefaultConfig::get();

        return std::make_shared<Configuration>(defaultConfiguartionValues);
    }
};