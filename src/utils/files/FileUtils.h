#pragma once

#include <string>
#include <vector>

class FileUtils {
public:
    static void createFile(const std::string& path, const std::string& name);

    static void createDirectory(const std::string& path, const std::string& name);

    static void writeLines(const std::string& path, const std::vector<std::string>& lines);

    static std::vector<std::string> readLines(const std::string& path);
};