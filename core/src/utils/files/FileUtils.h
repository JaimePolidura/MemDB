#pragma once

#include "shared.h"

class FileUtils {
public:
    static void createFile(const std::string& path, const std::string& name);

    static void createDirectory(const std::string& path, const std::string& name);

    static void writeLines(const std::string& path, const std::vector<std::string>& lines);

    static void appendBytes(const std::string& path, const std::vector<uint8_t>& bytes);

    static void appendLines(const std::string& path, const std::vector<std::string>& lines);

    static void clear(const std::string& path);

    static std::vector<std::string> readLines(const std::string& path);

    static std::vector<uint8_t> readBytes(const std::string& path);

    static bool exists(const std::string &path);

    static std::string getProgramBasePath(const std::string& programName);

    static std::string getFileInProgramBasePath(const std::string& programName, const std::string& fileName);

    static std::string getProgramsPath();

    static std::string getSeparator();
};