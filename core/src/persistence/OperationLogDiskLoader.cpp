#include "persistence/OperationLogDiskLoader.h"

std::vector<OperationBody> OperationLogDiskLoader::getAll() {
    if(!FileUtils::exists(FileUtils::getFileInProgramBasePath("memdb", this->oplogFileName))) {
        return std::vector<OperationBody>{};
    }

    std::vector<uint8_t> bytesFromOpLog = FileUtils::readBytes(FileUtils::getFileInProgramBasePath("memdb", this->oplogFileName));
    std::vector<OperationBody> logs = this->operationLogDeserializer.deserializeAll(bytesFromOpLog);

    return logs;
}

void OperationLogDiskLoader::writeToDisk(const std::vector<OperationBody>& toWrite) {
    std::vector<uint8_t> serialized = this->operationLogSerializer.serializeAll(toWrite);

    FileUtils::clear(FileUtils::getFileInProgramBasePath("memdb", this->oplogFileName));
    FileUtils::appendBytes(FileUtils::getFileInProgramBasePath("memdb", this->oplogFileName), serialized);
}
