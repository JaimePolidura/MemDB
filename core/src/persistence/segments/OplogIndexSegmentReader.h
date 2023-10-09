#pragma once

#include "shared.h"

#include "persistence/segments/serializers/OplogIndexSegmentDescriptorDeserializer.h"
#include "persistence/serializers/OperationLogDeserializer.h"
#include "persistence/segments/OplogIndexSegmentDescriptor.h"

#include "utils/files/FileUtils.h"
#include "messages/request/Request.h"

class OplogIndexSegmentReader {
private:
    OplogIndexSegmentDescriptorDeserializer oplogIndexSegmentDescriptorDeserializer{};
    OperationLogDeserializer operationLogDeserializer{};

    const std::string fullPathIndex;
    const std::string fullPathData;
    const std::string indexFileName;
    const std::string partitionPath;

public:
    OplogIndexSegmentReader(const std::string& fullPathIndex,
                            const std::string& fullPathData,
                            const std::string& indexFileName,
                            const std::string& partitionPath);

    OplogIndexSegmentDescriptor readIndexAt(uint64_t ptr);

    std::vector<OplogIndexSegmentDescriptor> readAllIndexFrom(uint64_t ptr);

    std::vector<OplogIndexSegmentDescriptor> readAllIndex();

    std::vector<uint8_t> readBytesDataByDescriptor(OplogIndexSegmentDescriptor descriptor);
};