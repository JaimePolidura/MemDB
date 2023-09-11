#include "persistence/OperationLogDiskLoader.h"

std::vector<OperationBody> OperationLogDiskLoader::getAll() {
    if(!FileUtils::exists(
            FileUtils::getFileInPath(configuration->get(ConfigurationKeys::MEMDB_CORE_DATA_PATH), this->oplogFileName))) {
        return std::vector<OperationBody>{};
    }

    std::vector<uint8_t> bytesFromOpLog = FileUtils::readBytes(
            FileUtils::getFileInPath(configuration->get(ConfigurationKeys::MEMDB_CORE_DATA_PATH), this->oplogFileName));
    std::vector<OperationBody> logs = this->operationLogDeserializer.deserializeAll(bytesFromOpLog);

    return logs;
}

void OperationLogDiskLoader::writeToDisk(const std::vector<OperationBody>& toWrite) {
    std::vector<uint8_t> serialized = this->operationLogSerializer.serializeAll(toWrite);

    FileUtils::clear(FileUtils::getFileInPath(configuration->get(ConfigurationKeys::MEMDB_CORE_DATA_PATH), this->oplogFileName));
    FileUtils::appendBytes(FileUtils::getFileInPath(configuration->get(ConfigurationKeys::MEMDB_CORE_DATA_PATH), this->oplogFileName), serialized);
}
