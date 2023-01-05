#include <Windows.h>
#include <stdexcept>
#include <fstream>

#include "FileUtils.h"

void FileUtils::createDirectory(const std::string &path, const std::string &name) {
    CreateDirectoryA((path + "\\" + name).c_str(), NULL);
}

void FileUtils::createFile(const std::string &path, const std::string &name) {
    HANDLE handle = CreateFile((path + "\\" + name).c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

    CloseHandle(handle);
}

void FileUtils::writeLines(const std::string &path, const std::vector<std::string> &lines) {
    std::ofstream output_file(path);

    if(!output_file.is_open())
        throw std::runtime_error("Cannot create configuration file");

    for (const std::string &line : lines)
        output_file << line << '\n';

    output_file.close();
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