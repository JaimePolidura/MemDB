#pragma once

#include "Configuration.h"
#include "keys/DefaultConfig.h"
#include "utils/files/FileUtils.h"
#include "utils/strings/StringUtils.h"

#include "shared.h"

class ConfiguartionLoader {
public:
    static configuration_t load() {
        const std::string configPath = FileUtils::getFileInProgramBasePath("memdb", "config.txt");
        std::vector<std::string> lines = FileUtils::readLines(configPath);
        std::map<std::string, std::string> defaultConfiguartionValues = DefaultConfig::get();

        std::map<std::string, std::string> configValues = lines.empty() ? createConfigurationFile(defaultConfiguartionValues) : readConfigFile(lines);

        if(!lines.empty())
            ConfiguartionLoader::writeConfigFileMissingDefaultConfig(configValues, defaultConfiguartionValues);

        return std::make_shared<Configuration>(configValues, defaultConfiguartionValues);
    }

private:
    static std::map<std::string, std::string> readConfigFile(const std::vector<std::string>& lines) {
        std::map<std::string, std::string> configValues{};

        for (const std::string &line: lines) {
            std::vector<std::string> splited = StringUtils::split(line, '=');

            if(splited.size() < 2)
                throw std::runtime_error("Error while parsing configuration line at " + line);

            std::string key = splited.at(0);
            std::string value = splited.at(1);

            configValues[key] = value;
        }

        return configValues;
    }

    static std::map<std::string, std::string> createConfigurationFile(const std::map<std::string, std::string>& defaultConfigValues) {
        std::vector<std::string> lines = parseConfigToLines(defaultConfigValues);

        FileUtils::createDirectory(FileUtils::getProgramsPath(), "memdb");
        FileUtils::createFile(FileUtils::getProgramBasePath("memdb"), "config.txt");
        FileUtils::writeLines(FileUtils::getFileInProgramBasePath("memdb", "config.txt"), lines);

        return defaultConfigValues;
    }

    static std::vector<std::string> parseConfigToLines(const std::map<std::string, std::string>& configValues) {
        std::vector<std::string> lines;
        lines.reserve(configValues.size());

        for (auto const& [key, value] : configValues)
            lines.push_back(key + "=" + value);

        return lines;
    }

    static std::map<std::string, std::string> parseLinesToConfig(const std::vector<std::string>& lines) {
        std::map<std::string, std::string> configValues{};

        for (const std::string &line : lines) {
            if(line == "" || line.size() == 0 || (* line.data()) == '\n')
                break;

            std::vector<std::string> splited = StringUtils::split(line, '=');

            if(splited.size() < 2)
                throw std::runtime_error("Error while parsing configuration line at " + line);

            std::string key = splited.at(0);
            std::string value = splited.at(1);

            configValues[key] = value;
        }

        return configValues;
    }

    static void writeConfigFileMissingDefaultConfig(const std::map<std::string, std::string>& configFromFile, const std::map<std::string, std::string>& defaultConfig) {
        std::vector<std::string> missingValuesToWrite{};

        for(auto const& [key, value] : defaultConfig)
            if(!configFromFile.contains(key))
                missingValuesToWrite.push_back(key + "=" + value);

        FileUtils::appendLines(FileUtils::getFileInProgramBasePath("memdb", "config.txt"), missingValuesToWrite);
    }
};