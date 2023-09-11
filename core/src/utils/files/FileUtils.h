#pragma once

#include "shared.h"

class FileUtils {
public:
    static void createFile(const std::string& path, const std::string& name);

    static void createDirectory(const std::string& path);

    static void appendBytes(const std::string& path, const std::vector<uint8_t>& bytes);

    static void writeBytes(const std::string& path, const std::vector<uint8_t>& bytes);

    static void clear(const std::string& path);

    static std::vector<uint8_t> readBytes(const std::string& path);

    static bool exists(const std::string &path);

    static std::string getFileInPath(const std::string& basePath, const std::string& fileName);
};