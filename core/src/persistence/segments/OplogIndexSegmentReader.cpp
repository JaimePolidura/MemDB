#include "OplogIndexSegmentReader.h"


OplogIndexSegmentReader::OplogIndexSegmentReader(const std::string &fullPathIndex, const std::string &fullPathData,
                                                 const std::string& indexFileName, const std::string& partitionPath, 
                                                 logger_t logger):
    fullPathIndex(fullPathIndex),
    fullPathData(fullPathData),
    indexFileName(indexFileName),
    partitionPath(partitionPath) {}

OplogIndexSegmentDescriptor OplogIndexSegmentReader::readIndexAt(uint64_t ptr) {
    return this->oplogIndexSegmentDescriptorDeserializer.deserialize(
            FileUtils::seekBytes(this->fullPathIndex, ptr, sizeof(OplogIndexSegmentDescriptor))
    );
}

std::vector<OplogIndexSegmentDescriptor> OplogIndexSegmentReader::readAllIndexBetween(uint64_t fromPtr, uint64_t toPtr) {
    if(fromPtr >= toPtr || (toPtr - fromPtr) < sizeof(OplogIndexSegmentDescriptor)){
        return std::vector<OplogIndexSegmentDescriptor>{};
    }

    toPtr += sizeof(OplogIndexSegmentDescriptor);
    uint64_t totalToRead = toPtr - fromPtr;

    std::vector<uint8_t> bytes = FileUtils::seekBytes(this->fullPathIndex, fromPtr, totalToRead);

    return this->oplogIndexSegmentDescriptorDeserializer.deserializeAll(bytes);
}

std::vector<OplogIndexSegmentDescriptor> OplogIndexSegmentReader::readAllIndexFrom(uint64_t ptr) {
    uint64_t totalToRead = FileUtils::size(this->partitionPath, this->indexFileName) - ptr;
    std::vector<uint8_t> bytes = FileUtils::seekBytes(this->fullPathIndex, ptr, totalToRead);

    return this->oplogIndexSegmentDescriptorDeserializer.deserializeAll(bytes);
}

std::vector<OplogIndexSegmentDescriptor> OplogIndexSegmentReader::readAllIndex() {
    return this->oplogIndexSegmentDescriptorDeserializer.deserializeAll(FileUtils::readBytes(this->fullPathIndex));
}

std::result<std::vector<uint8_t>> OplogIndexSegmentReader::readBytesDataByDescriptor(OplogIndexSegmentDescriptor descriptor) {
    auto bytes = FileUtils::seekBytes(this->fullPathData, descriptor.ptr, descriptor.size);

    if(Utils::crc(bytes) == descriptor.crc) {
        return std::result<std::vector<uint8_t>>::ok(bytes);
    } else {
        return std::result<std::vector<uint8_t>>::error();
    }
}
