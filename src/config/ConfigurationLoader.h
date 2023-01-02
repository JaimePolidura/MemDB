#pragma once

#include "Configuration.h"
#include "../utils/files/FileReader.h"
#include "../utils/files/FileWriter.h"
#include "../utils/strings/StringUtils.h"

#include <memory>
#include <vector>

class ConfiguartionLoader {
public:
    std::shared_ptr<Configuration> load() {
        const std::string configPath = this->getMemDbPath() + "/config.txt";
        std::vector<std::string> lines = FileReader::readFileLines(configPath);

        std::map<std::string, std::string> configValues = lines.empty() ? createConfigurationFile() : readConfigFile(lines);

        return std::make_shared<Configuration>(configValues);
    }

private:
    std::map<std::string, std::string> readConfigFile(const std::vector<std::string>& lines) {
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

    std::map<std::string, std::string> createConfigurationFile() {
        std::vector<std::string> lines = FileReader::readFileLines("DefaultConfig.txt");
        std::map<std::string, std::string> configValues = this->parseLinesToConfig(lines);

        FileWriter::writeLines(this->getMemDbPath() + "/config.txt", lines);

        return configValues;
    }

    std::map<std::string, std::string> parseLinesToConfig(const std::vector<std::string>& lines) {
        std::map<std::string, std::string> configValues{};

        for (const std::string &line : lines) {
            std::vector<std::string> splited = StringUtils::split(line, '=');

            if(splited.size() < 2)
                throw std::runtime_error("Error while parsing configuration line at " + line);

            std::string key = splited.at(0);
            std::string value = splited.at(1);

            configValues[key] = value;
        }

        return configValues;
    }

    std::string getMemDbPath() {
        #ifdef _WIN32
            return "C:/memdb";
        #else
            return "/etc/memdb";
        #endif
    }
};