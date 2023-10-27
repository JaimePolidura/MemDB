#pragma once

#include "shared.h"

#include "persistence/segments/serializers/OplogIndexSegmentDescriptorDeserializer.h"
#include "persistence/serializers/OperationLogDeserializer.h"
#include "persistence/segments/OplogIndexSegmentDescriptor.h"

#include "utils/files/FileUtils.h"
#include "utils/compression/Compressor.h"
#include "utils/std/Result.h"
#include "messages/request/Request.h"
#include "logging/Logger.h"

class OplogIndexSegmentReader {
private:
    OplogIndexSegmentDescriptorDeserializer oplogIndexSegmentDescriptorDeserializer{};
    OperationLogDeserializer operationLogDeserializer{};
    Compressor compressor{};
    logger_t logger;

    const std::string fullPathIndex;
    const std::string fullPathData;
    const std::string indexFileName;
    const std::string partitionPath;

public:
    OplogIndexSegmentReader(const std::string& fullPathIndex,
                            const std::string& fullPathData,
                            const std::string& indexFileName,
                            const std::string& partitionPath,
                            logger_t logger);

    OplogIndexSegmentDescriptor readIndexAt(uint64_t ptr);

    std::vector<OplogIndexSegmentDescriptor> readAllIndexBetween(uint64_t fromPtr, uint64_t toPtr);

    std::vector<OplogIndexSegmentDescriptor> readAllIndexFrom(uint64_t ptr);

    std::vector<OplogIndexSegmentDescriptor> readAllIndex();
    
    std::result<std::vector<uint8_t>> readBytesDataByDescriptor(OplogIndexSegmentDescriptor descriptor);
};