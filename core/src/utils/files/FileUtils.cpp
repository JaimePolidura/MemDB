#include "FileUtils.h"

#include <stdexcept>
#include <fstream>
#include <filesystem>

#ifdef _WIN32
    #include <Windows.h>
#else
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <fcntl.h>
#endif

void FileUtils::createDirectory(const std::string &path, const std::string &name) {
#ifdef _WIN32
    CreateDirectoryA((path + "\\" + name).c_str(), NULL);
#else
    std::string fullPath = path + "/" + name;
    mkdir(fullPath.c_str(), 0700);
#endif
}

void FileUtils::createFile(const std::string &path, const std::string &name) {
#ifdef _WIN32
    HANDLE handle = CreateFile((path + "\\" + name).c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

    CloseHandle(handle);
#else
    std::string fullPath = path + "/" + name;
    int fd = open(fullPath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0600);
    close(fd);
#endif
}

void FileUtils::appendLines(const std::string &path, const std::vector<std::string> &lines) {
    std::fstream file;
    file.open(path, std::ios::out | std::ios::app);

    for(auto line = lines.begin(); line < lines.end(); line++)
        file << * line;

    file.close();
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

bool FileUtils::exists(const std::string &path) {
    return std::filesystem::exists(path);
}

std::string FileUtils::getSeparator() {
    return std::string(std::filesystem::path::preferred_separator, 1);
}

void FileUtils::clear(const std::string &path) {
    std::ofstream myfile;
    myfile.open(path, std::ios::trunc);
    myfile.close();
}

std::vector<uint8_t> FileUtils::readBytes(const std::string &path) {
    std::ifstream file(path, std::ios::binary);

    if(!file.is_open())
        return {};

    std::vector<uint8_t> contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    return contents;
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