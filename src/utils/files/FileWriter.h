#pragma once

#include <string>
#include <fstream>
#include <fstream>
#include <vector>

class FileWriter {
public:
    static void writeLines(const std::string& file, const std::vector<std::string> lines) {
        std::ofstream output_file(file);

        if(output_file.is_open())
            throw std::runtime_error("Cannot create configuration file");

        for (const std::string &line : lines)
            output_file << line << '\n';

        output_file.close();
    }
};