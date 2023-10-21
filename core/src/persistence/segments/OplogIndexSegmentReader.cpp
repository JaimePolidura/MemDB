#include "OplogIndexSegmentReader.h"


OplogIndexSegmentReader::OplogIndexSegmentReader(const std::string &fullPathIndex, const std::string &fullPathData,
                                                 const std::string& indexFileName, const std::string& partitionPath):
    fullPathIndex(fullPathIndex),
    fullPathData(fullPathData),
    indexFileName(indexFileName),
    partitionPath(partitionPath) {}

OplogIndexSegmentDescriptor OplogIndexSegmentReader::readIndexAt(uint64_t ptr) {
    return this->oplogIndexSegmentDescriptorDeserializer.deserialize(
            FileUtils::seekBytes(this->fullPathIndex, ptr, sizeof(OplogIndexSegmentDescriptor))
    );
}

std::vector<OplogIndexSegmentDescriptor> OplogIndexSegmentReader::readAllIndexFrom(uint64_t ptr) {
    uint64_t totalToRead = FileUtils::size(this->partitionPath, this->indexFileName) - ptr;
    std::vector<uint8_t> bytes = FileUtils::seekBytes(this->fullPathIndex, ptr, totalToRead);

    return this->oplogIndexSegmentDescriptorDeserializer.deserializeAll(bytes);
}

std::vector<OplogIndexSegmentDescriptor> OplogIndexSegmentReader::readAllIndex() {
    return this->oplogIndexSegmentDescriptorDeserializer.deserializeAll(FileUtils::readBytes(this->fullPathIndex));
}

std::vector<uint8_t> OplogIndexSegmentReader::readBytesDataByDescriptor(OplogIndexSegmentDescriptor descriptor) {
    return FileUtils::seekBytes(this->fullPathData, descriptor.ptr, descriptor.size);
}
