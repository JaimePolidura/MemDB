#pragma once

#include "persistence/segments/serializers/OplogIndexSegmentDescriptorSerializer.h"
#include "persistence/compaction/OperationLogCompacter.h"
#include "persistence/serializers/OperationLogDeserializer.h"
#include "persistence/serializers/OperationLogSerializer.h"
#include "OplogIndexSegmentDescriptor.h"

#include "messages/request/Request.h"
#include "utils/files/FileUtils.h"
#include "utils/compression/Compressor.h"
#include "logging/Logger.h"

class OplogIndexSegmentWriter {
private:
    const std::string partitionPath;
    const std::string dataFileName;
    const std::string indexFileName;

    OplogIndexSegmentDescriptorSerializer indexSegmentDescSerializer;
    OperationLogDeserializer deserializer;
    OperationLogSerializer oplogSerializer;
    OperationLogCompacter compacter;
    Compressor compressor;
    logger_t logger;

    std::mutex writeLock;

public:
    OplogIndexSegmentWriter(const std::string& partitionPath, const std::string& dataFileName, const std::string& indexFileName, logger_t logger);

    void write(const std::vector<uint8_t>& toWrite);

    void updateCorruptedSegment(uint32_t uncompressedSize, uint64_t indexSegmentDescriptorPtr, OplogIndexSegmentDescriptor corruptedDescriptor, const std::vector<uint8_t>& uncorruptedBytes);
};
