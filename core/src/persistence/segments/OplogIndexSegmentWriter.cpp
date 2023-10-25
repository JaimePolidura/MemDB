#include "OplogIndexSegmentWriter.h"

OplogIndexSegmentWriter::OplogIndexSegmentWriter(const std::string& partitionPath, const std::string& dataFileName, const std::string& indexFileName, logger_t loggerCons):
    indexFileName(indexFileName),
    partitionPath(partitionPath),
    dataFileName(dataFileName),
    logger(loggerCons) {}

void OplogIndexSegmentWriter::write(const std::vector<uint8_t> &toWrite) {
    std::vector<OperationBody> operations = this->deserializer.deserializeAll(toWrite);
    std::vector<OperationBody> compacted = this->compacter.compact(operations);
    std::sort(compacted.begin(), compacted.end(), [](const OperationBody& a, const OperationBody& b){return a.timestamp < b.timestamp;});
    std::vector<uint8_t> compactedSerialized = this->oplogSerializer.serializeAll(compacted);
    std::vector<uint8_t> compactedCompressedSerialized = this->compressor.compressBytes(compactedSerialized).get_or_throw_with([](const int errorCode){
        return "Impossible to compress oplog in OplogIndexSegmentWriter::write. Error code: " + errorCode;
    });

    writeLock.lock();
    
    std::vector<uint8_t> descriptorSerialized = this->indexSegmentDescSerializer.serialize(OplogIndexSegmentDescriptor{
            .min = compacted.at(0).timestamp,
            .max = compacted.at(compacted.size() - 1).timestamp,
            .ptr = FileUtils::size(partitionPath, this->dataFileName),
            .crc = Utils::crc(compactedCompressedSerialized),
            .size = static_cast<uint32_t>(compactedCompressedSerialized.size()),
            .originalSize = static_cast<uint32_t>(compactedSerialized.size()),
            .flags = 0x00,
    });

    FileUtils::appendBytes(this->partitionPath + "/" + this->dataFileName, compactedCompressedSerialized);
    FileUtils::appendBytes(this->partitionPath + "/" + this->indexFileName, descriptorSerialized);

    writeLock.unlock();
}