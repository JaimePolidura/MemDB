#pragma once

#include <vector>
#include <mutex>

#include "utils/files/FileUtils.h"
#include "OperationLogSerializer.h"

class OperationsLogDiskWriter {
private:
    OperationLogSerializer operationLogSerializer;
    std::mutex writeFileLock;

public:
    void write(const std::vector<OperationBody>& toWrite) {
        this->createFileIfNotExistd();

        std::vector<uint8_t> serialized = this->serializeAll(toWrite);

        this->writeAppendModeSerialized(serialized);
    }

private:
    void writeAppendModeSerialized(const std::vector<uint8_t>& serialized) {
        writeFileLock.lock();

        FileUtils::appendBytes(FileUtils::getFileInProgramBasePath("memdb", "oplog"), serialized);

        writeFileLock.unlock();
    }

    void createFileIfNotExistd() {
        bool exists = FileUtils::exists(FileUtils::getFileInProgramBasePath("memdb", "oplog"));
        if(!exists)
            FileUtils::createFile(FileUtils::getProgramBasePath("memdb"), "oplog");
    }

    std::vector<uint8_t> serializeAll(const std::vector<OperationBody>& toSerialize) {
        std::vector<uint8_t> serialized{};
        for (auto actualOperation = toSerialize.begin(); actualOperation < toSerialize.end(); actualOperation++)
            this->operationLogSerializer.serialize(serialized, *actualOperation);

        return serialized;
    }
};