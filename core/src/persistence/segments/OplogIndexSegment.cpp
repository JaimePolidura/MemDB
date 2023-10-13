#include "OplogIndexSegment.h"

const std::string OplogIndexSegment::INDEX_FILE_NAME = "oplog-index";
const std::string OplogIndexSegment::DATA_FILE_NAME = "oplog-data";

OplogIndexSegment::OplogIndexSegment(configuration_t configuration, uint32_t oplogId):
        configuration(configuration),
        memdDbBasePath(configuration->get(ConfigurationKeys::DATA_PATH)),
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

std::vector<uint8_t> OplogIndexSegment::getDataByDescriptorBytes(OplogIndexSegmentDescriptor descriptor) {
    return this->oplogIndexSegmentReader.readBytesDataByDescriptor(descriptor);
}

void OplogIndexSegment::clearAll() {
    FileUtils::clear(this->fullPathIndex);
    FileUtils::clear(this->fullPathData);
}

void OplogIndexSegment::initializeFiles() {
    if(!FileUtils::exists(fullPathIndex)){
        FileUtils::createFile(partitionPath, OplogIndexSegment::INDEX_FILE_NAME);
    }
    if(!FileUtils::exists(fullPathData)){
        FileUtils::createFile(partitionPath, OplogIndexSegment::DATA_FILE_NAME);
    }
}