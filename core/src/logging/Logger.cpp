#include "logging/Logger.h"

Logger::Logger(configuration_t configuration, const std::string& intialMessage):
    debugLogger(spdlog::create<spdlog::sinks::stdout_color_sink_mt>("debug")),
    infoLogger(spdlog::create<spdlog::sinks::stdout_color_sink_mt>("info")),
    configuration(configuration) {

    this->debugLogger->set_pattern("[%H:%M:%S] [" + configuration->get(ConfigurationKeys::NODE_ID) + "] [DEBUG] %v");
    this->debugLogger->set_level(spdlog::level::debug);

    this->infoLogger->set_pattern("[%H:%M:%S] [" + configuration->get(ConfigurationKeys::NODE_ID) + "] [INFO] %v");
    this->infoLogger->set_level(spdlog::level::info);

    this->infoLogger->info(intialMessage);
}