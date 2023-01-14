#include "FileUtils.h"

#include <stdexcept>
#include <fstream>
#include <filesystem>

#ifdef _WIN32
    #include <Windows.h>
#endif

void FileUtils::createDirectory(const std::string &path, const std::string &name) {
#ifdef _WIN32
    CreateDirectoryA((path + "\\" + name).c_str(), NULL);
#else
    throw std::logic_error("Not implemented");
#endif
}

void FileUtils::createFile(const std::string &path, const std::string &name) {
#ifdef _WIN32
    HANDLE handle = CreateFile((path + "\\" + name).c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

    CloseHandle(handle);
#else
    throw std::logic_error("Not implemented");
#endif
}

void FileUtils::appendBytes(const std::string &path, const std::vector<uint8_t> &bytes) {
    std::fstream file;
    file.open(path, std::ios::out | std::ios::app);

    for(auto byte = bytes.begin(); byte < bytes.end(); byte++)
        file << * byte;

    file.close();
}

std::string FileUtils::getProgramsPath() {
#ifdef _WIN32
    return "C:";
#else
    return "/etc";
#endif
}

std::string FileUtils::getFileInProgramBasePath(const std::string &programName, const std::string &fileName) {
#ifdef _WIN32
    return "C:\\" + programName + "\\" + fileName;
#else
    return "/etc/" + programName + "/" + fileName;
#endif
}

std::string FileUtils::getProgramBasePath(const std::string &programName) {
#ifdef _WIN32
    return "C:\\" + programName;
#else
    return "/etc/" + programName;
#endif
}

bool FileUtils::exists(const std::string &path, const std::string &name) {
    return std::filesystem::exists(path + getSeparator() + name);
}

std::string FileUtils::getSeparator() {
    return std::string(std::filesystem::path::preferred_separator, 1);
}

std::vector<uint8_t> FileUtils::readBytes(const std::string &path) {
    std::vector<uint8_t> bytes{};
    std::ifstream file(path, std::ios::binary);

    if(!file.is_open())
        return bytes;

    std::vector<uint8_t> contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    return bytes;
}

std::vector<std::string> FileUtils::readLines(const std::string &path) {
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

void FileUtils::writeLines(const std::string &path, const std::vector<std::string> &lines) {
    std::ofstream output_file(path);

    if(!output_file.is_open())
        throw std::runtime_error("Cannot create configuration file");

    for (const std::string &line : lines)
        output_file << line << '\n';

    output_file.close();
}