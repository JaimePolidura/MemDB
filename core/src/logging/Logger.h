#pragma once

#include "config/Configuration.h"
#include "shared.h"
#include "memdbtypes.h"

class Logger {
private:
    std::shared_ptr<spdlog::logger> infoLogger;
    std::shared_ptr<spdlog::logger> debugLogger;
    configuration_t configuration;

public:
    Logger(configuration_t configuration, const std::string& intialMessage);

    template<typename... Args>
    inline void info(fmt::format_string<Args...> fmt, Args &&... args) {
        this->infoLogger->info(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void error(fmt::format_string<Args...> fmt, Args &&... args) {
        this->infoLogger->error(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void warn(fmt::format_string<Args...> fmt, Args &&... args) {
        this->infoLogger->warn(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void debugInfo(fmt::format_string<Args...> fmt, Args &&... args) {
        if(this->configuration->getBoolean(ConfigurationKeys::SHOW_DEBUG_LOG)){
            this->debugLogger->info(fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    inline void errorInfo(fmt::format_string<Args...> fmt, Args &&... args) {
        if(this->configuration->getBoolean(ConfigurationKeys::SHOW_DEBUG_LOG)){
            this->debugLogger->error(fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    inline void warnInfo(fmt::format_string<Args...> fmt, Args &&... args) {
        if(this->configuration->getBoolean(ConfigurationKeys::SHOW_DEBUG_LOG)){
            this->debugLogger->warn(fmt, std::forward<Args>(args)...);
        }
    }
};

using logger_t = std::shared_ptr<Logger>;