#include "FileUtils.h"

#include "shared.h"

#ifdef _WIN32
    #include <Windows.h>
#else
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <fcntl.h>
#endif

void FileUtils::writeBytes(const std::string &path, const std::vector<uint8_t> &bytes) {
    std::ofstream file(path, std::ios::trunc | std::ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        file.flush();
        file.close();
    }else{
        throw std::runtime_error("Cannot open filePath " + path);
    }
}

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
    int fd = open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0600);
    close(fd);
#endif
}

void FileUtils::appendBytes(const std::string &path, const std::vector<uint8_t> &bytes) {
    std::fstream file;
    file.open(path, std::ios::out | std::ios::app);

    for(auto byte = bytes.begin(); byte < bytes.end(); byte++)
        file << * byte;

    file.flush();
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
    try{
        return std::filesystem::exists(path);
    }catch(const std::exception& e) {
        return false;
    }
}

std::string FileUtils::getSeparator() {
    return std::string(std::filesystem::path::preferred_separator, 1);
}

void FileUtils::clear(const std::string &path) {
    std::ofstream myfile;
    myfile.open(path, std::ios::trunc);
    myfile.close();
    std::remove(path.data());
}

std::vector<uint8_t> FileUtils::readBytes(const std::string &path) {
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open())
        return {};

    std::vector<uint8_t> contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    return contents;
}