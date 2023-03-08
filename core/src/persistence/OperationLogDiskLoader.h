#pragma once

#include "utils/files/FileUtils.h"
#include "utils/datastructures/map/Map.h"
#include "OperationLogDeserializer.h"
#include "operators/operations/SetOperator.h"
#include "persistence/compaction/SingleThreadedLogCompacter.h"+
#include "persistence/OperationLogSerializer.h"

class OperationLogDiskLoader {
private:
    OperationLogDeserializer operationLogDeserializer;
    SingleThreadedLogCompacter operationLogCompater;
    OperationLogSerializer operationLogSerializer;

public:
    std::vector<OperationBody> getAll() {
        if(!FileUtils::exists(FileUtils::getFileInProgramBasePath("memdb", "oplog")))
            return std::vector<OperationBody>{};

        std::vector<uint8_t> bytesFromOpLog = FileUtils::readBytes(FileUtils::getFileInProgramBasePath("memdb", "oplog"));
        std::vector<OperationBody> logs = this->operationLogDeserializer.deserializeAll(bytesFromOpLog);

        return logs;
    }

    std::vector<OperationBody> getAllAndSaveCompacted(memDbDataStore_t db) {
        if(!FileUtils::exists(FileUtils::getFileInProgramBasePath("memdb", "oplog")))
            return std::vector<OperationBody>{};

        std::vector<uint8_t> bytesFromOpLog = FileUtils::readBytes(FileUtils::getFileInProgramBasePath("memdb", "oplog"));
        std::vector<OperationBody> unCompactedLogs = this->operationLogDeserializer.deserializeAll(bytesFromOpLog);
        std::vector<OperationBody> compactedLogs = this->operationLogCompater.compact(unCompactedLogs);

        this->writeToDisk(compactedLogs);

        return compactedLogs;
    }

private:
    void writeToDisk(const std::vector<OperationBody>& toWrite) {
        std::vector<uint8_t> serialized = this->operationLogSerializer.serializeAll(toWrite);

        FileUtils::clear(FileUtils::getFileInProgramBasePath("memdb", "oplog"));
        FileUtils::appendBytes(FileUtils::getFileInProgramBasePath("memdb", "oplog"), serialized);
    }
};