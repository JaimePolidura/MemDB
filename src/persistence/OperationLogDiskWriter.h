#pragma once

#include <vector>
#include <mutex>

#include "utils/files/FileUtils.h"
#include "OperationLog.h"
#include "OperationLogSerializer.h"

class OperationsLogDiskWriter {
private:
    OperationLogSerializer operationLogSerializer;
    std::mutex writeFileLock;

public:
    void write(const std::vector<OperationLog>& toWrite) {
        this->createFileIfNotExistd();

        std::vector<uint8_t> serialized = this->serializeAll(toWrite);

        this->appendSerialized(serialized);
    }

private:
    void appendSerialized(const std::vector<uint8_t>& serialized) {
        writeFileLock.lock();

        FileUtils::appendBytes(FileUtils::getFileInProgramBasePath("memdb", "oplog.data"), serialized);

        writeFileLock.unlock();
    }

    void createFileIfNotExistd() {
        bool exists = FileUtils::exists(FileUtils::getProgramBasePath("memdb"), "oplog.data");
        if(!exists)
            FileUtils::createFile(FileUtils::getProgramBasePath("memdb"), "oplog.data");
    }

    std::vector<uint8_t> serializeAll(const std::vector<OperationLog>& toSerialize) {
        std::vector<uint8_t> serialized{};
        for (auto actualOperation = toSerialize.begin(); actualOperation < toSerialize.end(); actualOperation++)
            operationLogSerializer.serialize(serialized, *actualOperation);

        return serialized;
    }
};