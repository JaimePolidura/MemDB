#pragma once

#include "shared.h"

#include "utils/files/FileUtils.h"
#include "persistence/OperationLogSerializer.h"

class OperationsLogDiskWriter {
private:
    OperationLogSerializer operationLogSerializer;
    std::recursive_mutex writeFileLock;
    bool fileCreated = false;
    bool fileCleared = false;
    std::string oplogFileName;

public:
    OperationsLogDiskWriter(const std::string& oplogFileName): oplogFileName(oplogFileName) {}

    void clear() {
        writeFileLock.lock();
        FileUtils::clear(FileUtils::getFileInProgramBasePath("memdb", this->oplogFileName));
        this->fileCleared = true;
        writeFileLock.unlock();
    }

    void write(const std::vector<OperationBody>& toWrite) {
        this->createFileIfNotExists();

        std::vector<uint8_t> serialized = this->serializeAll(toWrite);

        writeFileLock.lock();
        if(this->fileCleared) return;

        FileUtils::writeBytes(FileUtils::getFileInProgramBasePath("memdb", this->oplogFileName), serialized);
        writeFileLock.unlock();
    }

    void append(const std::vector<OperationBody>& toWrite) {
        this->createFileIfNotExists();

        std::vector<uint8_t> serialized = this->serializeAll(toWrite);
        if(this->fileCleared) return;

        writeFileLock.lock();
        FileUtils::appendBytes(FileUtils::getFileInProgramBasePath("memdb", this->oplogFileName), serialized);
        writeFileLock.unlock();
    }

    void lockWrites() {
        this->writeFileLock.lock();
    }

    void unlockWrites() {
        this->writeFileLock.unlock();
    }

private:
    void createFileIfNotExists() {
        if(this->fileCreated)
            return;

        bool exists = FileUtils::exists(FileUtils::getFileInProgramBasePath("memdb", this->oplogFileName));
        if(!exists) {
            if(!FileUtils::exists(FileUtils::getProgramsPath() + "/memdb"))
                FileUtils::createDirectory(FileUtils::getProgramsPath(), "memdb");

            FileUtils::createFile(FileUtils::getProgramBasePath("memdb"), this->oplogFileName);
        }

        this->fileCreated = true;
    }

    std::vector<uint8_t> serializeAll(const std::vector<OperationBody>& toSerialize) {
        std::vector<uint8_t> serialized{};
        for (auto actualOperation = toSerialize.begin(); actualOperation < toSerialize.end(); actualOperation++)
            this->operationLogSerializer.serialize(serialized, *actualOperation);

        return serialized;
    }
};