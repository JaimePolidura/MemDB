#include "IntermediateOplog.h"

const std::string IntermediateOplog::FILE_NAME = "oplog-intermediate";

IntermediateOplog::IntermediateOplog(configuration_t configuration, uint32_t oplogId):
    configuration(configuration),
    memdDbBasePath(configuration->get(ConfigurationKeys::MEMDB_CORE_DATA_PATH)),
    partitionPath(memdDbBasePath + "/" + std::to_string(oplogId)),
    fullPath(partitionPath + "/" + FILE_NAME) {
    this->initializeFiles();
}

void IntermediateOplog::add(const OperationBody& operation) {
    this->addAll({operation});
}

void IntermediateOplog::addAll(const std::vector<OperationBody>& operations) {
    std::vector<uint8_t> serialized = this->operationLogSerializer.serializeAll(operations);
    FileUtils::appendBytes(this->fullPath, serialized);
    this->bytesWritten += serialized.size();

    if(this->bytesWritten > this->configuration->get<uint64_t>(ConfigurationKeys::PERSISTENCE_SEGMENT_SIZE_MB) * 1024 * 1024){
        this->bytesWritten = 0;
        this->onFlushingIntermediate(FileUtils::readBytes(fullPath));
        FileUtils::clear(this->fullPath);
    }
}

std::vector<uint8_t> IntermediateOplog::getAllBytes() {
    return FileUtils::readBytes(this->fullPath);
}

void IntermediateOplog::setOnFlushingIntermediate(std::function<void(const std::vector<uint8_t> &)> callback) {
    this->onFlushingIntermediate = callback;
}

void IntermediateOplog::clearAll() {
    FileUtils::clear(this->fullPath);
}

void IntermediateOplog::initializeFiles() {
    if(!FileUtils::exists(memdDbBasePath)){
        FileUtils::createDirectory(memdDbBasePath);
    }
    if(!FileUtils::exists(partitionPath)){
        FileUtils::createDirectory(partitionPath);
    }
    if(!FileUtils::exists(fullPath)){
        FileUtils::createFile(partitionPath, IntermediateOplog::FILE_NAME);
    }
}