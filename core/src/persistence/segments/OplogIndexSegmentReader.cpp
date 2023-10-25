#include "OplogIndexSegmentReader.h"


OplogIndexSegmentReader::OplogIndexSegmentReader(const std::string &fullPathIndex, const std::string &fullPathData,
                                                 const std::string& indexFileName, const std::string& partitionPath, 
                                                 logger_t logger):
    fullPathIndex(fullPathIndex),
    fullPathData(fullPathData),
    indexFileName(indexFileName),
    oplogCompressor(logger),
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

std::vector<uint8_t> OplogIndexSegmentReader::readBytesDataByDescriptor(OplogIndexSegmentDescriptor descriptor, bool compressed) {
    std::vector<uint8_t> bytesFromDisk = FileUtils::seekBytes(this->fullPathData, descriptor.ptr, descriptor.size);

    if(compressed || descriptor.hasFlag(OplogIndexSegmentDescriptorFlag::UNCOMPRESSED)) {
        return bytesFromDisk;
    }

    return this->oplogCompressor.uncompressOplog(bytesFromDisk, descriptor.originalSize)
        .get_or_throw_with([](auto errorCode){return "Error while decompressing oplog from disk. Return code: " + std::to_string(errorCode);});
}
