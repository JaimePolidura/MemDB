#pragma once

#include "Configuration.h"
#include "keys/DefaultConfig.h"
#include "../utils/files/FIleUtilsInclude.h"
#include "../utils/strings/StringUtils.h"

#include <memory>
#include <vector>

class ConfiguartionLoader {
public:
    static std::shared_ptr<Configuration> load() {
        const std::string configPath = getMemDbPath() + "/config.txt";
        std::vector<std::string> lines = FileUtils::readLines(configPath);

        std::map<std::string, std::string> configValues = lines.empty() ? createConfigurationFile() : readConfigFile(lines);

        return std::make_shared<Configuration>(configValues);
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

    static std::map<std::string, std::string> createConfigurationFile() {
        std::map<std::string, std::string> defaultConfigValues = DefaultConfig::get();
        std::vector<std::string> lines = parseConfigToLines(defaultConfigValues);

        FileUtils::createDirectory(getBasePath(), "memdb");
        FileUtils::createFile(getMemDbPath(), "config.txt");
        FileUtils::writeLines(getMemDbConfigFolder(), lines);

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

    static std::string getMemDbConfigFolder() {
        #ifdef _WIN32
                return "C:\\memdb\\config.txt";
        #else
                return "/etc/memdb/config.txt";
        #endif
    }

    static std::string getBasePath() {
        #ifdef _WIN32
                return "C:";
        #else
                return "/etc";
        #endif
    }

    static std::string getMemDbPath() {
        #ifdef _WIN32
            return "C:\\memdb";
        #else
            return "/etc/memdb";
        #endif
    }
};