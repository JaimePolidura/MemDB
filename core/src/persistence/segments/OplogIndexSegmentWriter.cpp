#include "OplogIndexSegmentWriter.h"

OplogIndexSegmentWriter::OplogIndexSegmentWriter(const std::string& partitionPath, const std::string& dataFileName, const std::string& indexFileName):
    indexFileName(indexFileName),
    partitionPath(partitionPath),
    dataFileName(dataFileName) {}

void OplogIndexSegmentWriter::write(const std::vector<uint8_t> &toWrite) {
    std::vector<OperationBody> operations = this->deserializer.deserializeAll(toWrite);
    std::vector<OperationBody> compacted = this->compacter.compact(operations);
    std::sort(compacted.begin(), compacted.end(), [](const OperationBody& a, const OperationBody& b){return a.timestamp - b.timestamp;});
    std::vector<uint8_t> compactedSerialized = this->oplogSerializer.serializeAll(compacted);

    writeLock.lock();

    std::vector<uint8_t> descriptorSerialized = this->indexSegmentDescSerializer.serialize(OplogIndexSegmentDescriptor{
            .min = compacted.at(0).timestamp,
            .max = compacted.at(compacted.size() - 1).timestamp,
            .ptr = FileUtils::size(partitionPath, this->dataFileName),
            .crc = Utils::crc(compactedSerialized),
            .size = static_cast<uint32_t>(compactedSerialized.size())
    });

    FileUtils::appendBytes(this->partitionPath + "/" + this->indexFileName, descriptorSerialized);
    FileUtils::appendBytes(this->partitionPath + "/" + this->dataFileName, compactedSerialized);

    writeLock.unlock();
}