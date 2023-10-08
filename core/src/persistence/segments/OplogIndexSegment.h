#pragma once

#include "shared.h"

#include "OplogIndexSegmentWriter.h"
#include "OplogIndexSegmentReader.h"
#include "config/Configuration.h"
#include "utils/files/FileUtils.h"

class OplogIndexSegment {
private:
    configuration_t configuration;

    const std::string memdDbBasePath;
    const std::string partitionPath;
    const std::string fullPathIndex;
    const std::string fullPathData;

    OplogIndexSegmentWriter oplogIndexSegmentWriter;
    OplogIndexSegmentReader oplogIndexSegmentReader;

public:
    static const std::string INDEX_FILE_NAME;
    static const std::string DATA_FILE_NAME;

    OplogIndexSegment(configuration_t configuration, uint32_t oplogId);

    std::vector<OplogIndexSegmentDescriptor> getByAfterTimestamp(uint64_t timestamp);

    std::vector<OplogIndexSegmentDescriptor> getAll();

    std::vector<OperationBody> getDataByDescriptor(OplogIndexSegmentDescriptor descriptor);

    void save(const std::vector<uint8_t>& toSave);

private:
    void initializeFiles();
};

using oplogIndexSegment_t = std::shared_ptr<OplogIndexSegment>;
