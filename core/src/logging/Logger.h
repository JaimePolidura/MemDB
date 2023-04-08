#pragma once

#include "shared.h"

class Logger {
public:
    Logger() = default;

    Logger(const std::string& intialMessage) {
        spdlog::info(intialMessage);
    }

    template<typename... Args>
    inline void info(fmt::format_string<Args...> fmt, Args &&... args) {
        spdlog::info(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void error(fmt::format_string<Args...> fmt, Args &&... args) {
        spdlog::error(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void warn(fmt::format_string<Args...> fmt, Args &&... args) {
        spdlog::warn(fmt, std::forward<Args>(args)...);
    }
};

using logger_t = std::shared_ptr<Logger>;