#pragma once

#include "persistence/segments/serializers/OplogIndexSegmentDescriptorSerializer.h"
#include "persistence/compaction/OperationLogCompacter.h"
#include "persistence/serializers/OperationLogDeserializer.h"
#include "persistence/serializers/OperationLogSerializer.h"
#include "OplogIndexSegmentDescriptor.h"
#include "messages/request/Request.h"
#include "utils/files/FileUtils.h"

class OplogIndexSegmentWriter {
private:
    const std::string partitionPath;
    const std::string dataFileName;
    const std::string indexFileName;

    OplogIndexSegmentDescriptorSerializer indexSegmentDescSerializer;
    OperationLogDeserializer deserializer;
    OperationLogSerializer oplogSerializer;
    OperationLogCompacter compacter;
    std::mutex writeLock;

public:
    OplogIndexSegmentWriter(const std::string& partitionPath, const std::string& dataFileName, const std::string& indexFileName);

    void write(const std::vector<uint8_t>& toWrite);
};
