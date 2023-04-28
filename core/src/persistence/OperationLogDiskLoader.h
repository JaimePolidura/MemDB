#pragma once

#include "utils/files/FileUtils.h"
#include "utils/datastructures/map/Map.h"
#include "utils/Utils.h"

#include "persistence/OperationLogDeserializer.h"
#include "operators/operations/SetOperator.h"
#include "persistence/OperationLogSerializer.h"

class OperationLogDiskLoader {
private:
    OperationLogDeserializer operationLogDeserializer;
    OperationLogSerializer operationLogSerializer;
    std::string oplogFileName;

public:
    OperationLogDiskLoader(const std::string& oplogFileName): oplogFileName(oplogFileName) {}

    std::vector<OperationBody> getAll() {
        if(!FileUtils::exists(FileUtils::getFileInProgramBasePath("memdb", this->oplogFileName))) {
            return std::vector<OperationBody>{};
        }

        std::vector<uint8_t> bytesFromOpLog = FileUtils::readBytes(FileUtils::getFileInProgramBasePath("memdb", this->oplogFileName));
        std::vector<OperationBody> logs = this->operationLogDeserializer.deserializeAll(bytesFromOpLog);

        return logs;
    }

private:
    void writeToDisk(const std::vector<OperationBody>& toWrite) {
        std::vector<uint8_t> serialized = this->operationLogSerializer.serializeAll(toWrite);

        FileUtils::clear(FileUtils::getFileInProgramBasePath("memdb", this->oplogFileName));
        FileUtils::appendBytes(FileUtils::getFileInProgramBasePath("memdb", this->oplogFileName), serialized);
    }
};