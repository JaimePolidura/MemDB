#pragma once

#include <string>
#include <vector>
#include <fstream>

class FileReader {
public:
    static std::vector<std::string> readFileLines(const std::string& path) {
        std::vector<std::string> lines{};
        std::ifstream file(path);

        if(!file.is_open())
            return lines;

        std::string line;
        while(std::getline(file, line))
            lines.push_back(line);

        file.close();

        return lines;
    }
};