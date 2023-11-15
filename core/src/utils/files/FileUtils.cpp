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

void FileUtils::deleteTopBytes(const std::string& path, uint64_t nBytesToDelete) {
    std::fstream file(path, std::ios::in | std::ios::out | std::ios::binary);
    if(!file.is_open()) {
        throw std::runtime_error("Cannot open filePath " + path);
    }

    uint64_t fileSize = file.tellg();

    file.seekg(fileSize - nBytesToDelete, std::ios::beg);
    file.seekp(fileSize - nBytesToDelete, std::ios::beg);
    file.trunc(fileSize - nBytesToDelete);

    file.close();
}

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

void FileUtils::writeBytes(const std::string& path, uint64_t ptr, const std::vector<uint8_t>& bytes) {
    std::ofstream file(path, std::ios::trunc | std::ios::binary);
    if (file.is_open()) {
        file.seekp(ptr, std::ios::beg);
        file.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
        file.flush();
        file.close();
    } else {
        throw std::runtime_error("Cannot open filePath " + path);
    }
}

void FileUtils::createDirectory(const std::string &path) {
#ifdef _WIN32
    CreateDirectoryA(path.c_str(), NULL);
#else
    mkdir(path.c_str(), 0777);
#endif
}

std::vector<uint8_t> FileUtils::seekBytes(const std::string &path, uint64_t ptr, uint64_t total) {
    std::vector<uint8_t> bytes(total);
    uint8_t * toWrite = bytes.data();

    std::ifstream file(path, std::ios::binary);
    file.seekg(ptr);

    file.read((char *) toWrite, total);
    file.close();

    return bytes;
}

std::vector<std::string> FileUtils::ls(const std::string &path) {
    std::vector<std::string> filesNames{};

    for(auto const& file : std::filesystem::directory_iterator(path)){
        if(file.is_regular_file()){
            filesNames.push_back(file.path().filename().string());
        }
    }

    return filesNames;
}

uint64_t FileUtils::size(const std::string &basePath, const std::string &fileName) {
    std::ifstream in(basePath + "/" + fileName, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

void FileUtils::createFile(const std::string &path, const std::string &name) {
#ifdef _WIN32
    HANDLE handle = CreateFile((path + "\\" + name).c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

    CloseHandle(handle);
#else
    std::string fullPath = path + "/" + name;
    int fd = open(fullPath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if(fd < 0){
        throw std::runtime_error("Error when creating file: " + path + "/" + name + " Got return error: " + std::to_string(fd));
    }

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

std::string FileUtils::getFileInPath(const std::string &basePath, const std::string &fileName) {
#ifdef _WIN32
    return basePath + "\\" + fileName;
#else
    return basePath + "/" + fileName;
#endif
}

bool FileUtils::exists(const std::string &path) {
    try{
        return std::filesystem::exists(path);
    }catch(const std::exception& e) {
        return false;
    }
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