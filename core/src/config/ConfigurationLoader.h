#pragma once

#include "config/Configuration.h"
#include "config/keys/DefaultConfig.h"
#include "utils/files/FileUtils.h"
#include "utils/strings/StringUtils.h"
#include "utils/Utils.h"

#include "shared.h"

class ConfiguartionLoader {
public:
    static configuration_t load(int nArgs, char ** args);
};