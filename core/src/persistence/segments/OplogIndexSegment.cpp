#include "OplogIndexSegment.h"

OplogIndexSegment::OplogIndexSegment(configuration_t configuration, uint32_t oplogId):
        configuration(configuration),
        memdDbBasePath(configuration->get(ConfigurationKeys::MEMDB_CORE_DATA_PATH)),
        partitionPath(memdDbBasePath + "/" + std::to_string(oplogId)),
        fullPathIndex(partitionPath + "/" + INDEX_FILE_NAME),
        fullPathData(partitionPath + "/" + DATA_FILE_NAME),
        oplogIndexSegmentReader(fullPathIndex, fullPathData, INDEX_FILE_NAME, partitionPath),
        oplogIndexSegmentWriter(partitionPath, DATA_FILE_NAME, INDEX_FILE_NAME) {
    this->initializeFiles();
}

void OplogIndexSegment::save(const std::vector<uint8_t>& toSave) {
    this->oplogIndexSegmentWriter.write(toSave);
}

std::vector<OplogIndexSegmentDescriptor> OplogIndexSegment::getByAfterTimestamp(uint64_t timestamp) {
    uint32_t actualPtr = FileUtils::size(partitionPath, INDEX_FILE_NAME) / 2 - 1;
    OplogIndexSegmentDescriptor actualDesc = this->oplogIndexSegmentReader.readIndexAt(actualPtr);

    while(actualDesc.min < timestamp) {
        actualDesc = this->oplogIndexSegmentReader.readIndexAt(actualPtr);

        if(actualPtr == 0){
            return {};
        }

        actualPtr /= 2;
    }

    return this->oplogIndexSegmentReader.readAllIndexFrom(actualPtr);
}

std::vector<OplogIndexSegmentDescriptor> OplogIndexSegment::getAll() {
    return this->oplogIndexSegmentReader.readAllIndex();
}

std::vector<OperationBody> OplogIndexSegment::getDataByDescriptor(OplogIndexSegmentDescriptor descriptor) {
    return this->oplogIndexSegmentReader.readDataByDescriptor(descriptor);
}

void OplogIndexSegment::initializeFiles() {
    if(!FileUtils::exists(memdDbBasePath)){
        FileUtils::createDirectory(memdDbBasePath);
    }
    if(!FileUtils::exists(partitionPath)){
        FileUtils::createDirectory(partitionPath);
    }
    if(!FileUtils::exists(fullPathIndex)){
        FileUtils::createFile(partitionPath, OplogIndexSegment::INDEX_FILE_NAME);
    }
    if(!FileUtils::exists(fullPathData)){
        FileUtils::createFile(partitionPath, OplogIndexSegment::DATA_FILE_NAME);
    }
}