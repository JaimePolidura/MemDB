#include "persistence/OperationLogDiskWriter.h"

void OperationsLogDiskWriter::clear() {
    FileUtils::clear(FileUtils::getFileInPath(configuration->get(ConfigurationKeys::MEMDB_CORE_DATA_PATH), this->oplogFileName));
    this->fileCleared.store(true, std::memory_order_release);
}

void OperationsLogDiskWriter::append(const std::vector<OperationBody>& toWrite) {
    this->createFileIfNotExists();

    std::vector<uint8_t> serialized = this->serializeAll(toWrite);

    if(this->fileCleared.load(std::memory_order_acquire)) {
        return;
    }

    FileUtils::appendBytes(FileUtils::getFileInPath(configuration->get(ConfigurationKeys::MEMDB_CORE_DATA_PATH), this->oplogFileName), serialized);
}

void OperationsLogDiskWriter::createFileIfNotExists() {
    if(this->fileCreated) {
        return;
    }

    const std::string memdbFilePath = configuration->get(ConfigurationKeys::MEMDB_CORE_DATA_PATH);

    bool exists = FileUtils::exists(memdbFilePath);
    if(!exists) {
        if(!FileUtils::exists(memdbFilePath)) {
            FileUtils::createDirectory(memdbFilePath);
        }

        FileUtils::createFile(memdbFilePath, this->oplogFileName);
    }

    this->fileCreated = true;
}

std::vector<uint8_t> OperationsLogDiskWriter::serializeAll(const std::vector<OperationBody>& toSerialize) {
    std::vector<uint8_t> serialized{};
    for (auto actualOperation = toSerialize.begin(); actualOperation < toSerialize.end(); actualOperation++)
        this->operationLogSerializer.serialize(serialized, *actualOperation);

    return serialized;
}