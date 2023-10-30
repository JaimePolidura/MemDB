#pragma once

#include "shared.h"

class FileUtils {
public:
    static void createFile(const std::string& path, const std::string& name);

    static void createDirectory(const std::string& path);

    static void appendBytes(const std::string& path, const std::vector<uint8_t>& bytes);

    static void writeBytes(const std::string& path, const std::vector<uint8_t>& bytes);

    static void writeBytes(const std::string& path, uint64_t ptr, const std::vector<uint8_t>& bytes);

    static void clear(const std::string& path);

    static std::vector<uint8_t> readBytes(const std::string& path);

    static bool exists(const std::string &path);

    static std::vector<uint8_t> seekBytes(const std::string& path, uint64_t ptr, uint64_t total);

    static std::vector<std::string> ls(const std::string &path);

    static uint64_t size(const std::string& basePath, const std::string& fileName);

    static std::string getFileInPath(const std::string& basePath, const std::string& fileName);
};