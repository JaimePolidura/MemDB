#pragma once

#include "shared.h"

#include "OplogIndexSegmentWriter.h"
#include "OplogIndexSegmentReader.h"
#include "config/Configuration.h"
#include "utils/files/FileUtils.h"
#include "logging/Logger.h"
#include "utils/std/Result.h"

struct OplogSegmentAfterTimestampsSearchResult {
    std::vector<OplogIndexSegmentDescriptor> descriptors;
    uint64_t descriptorInitPtr;
};

struct OplogSegmentBetweenTimestampsSearchResult {
    bool resultsInDescriptorsAndIntermediate;
    bool resultsOnlyInIntermediate;
    bool resultsOnlyInSegments;
    uint64_t descriptorInitPtr;
    std::vector<OplogIndexSegmentDescriptor> descriptors;
};

struct OplogSegmentBinarySearchResult {
    bool found;
    uint64_t ptr;
};

class OplogIndexSegment {
private:
    configuration_t configuration;
    logger_t logger;

    const std::string memdDbBasePath;
    const std::string partitionPath;
    const std::string fullPathIndex;
    const std::string fullPathData;

    OplogIndexSegmentWriter oplogIndexSegmentWriter;
    OplogIndexSegmentReader oplogIndexSegmentReader;

public:
    static const std::string INDEX_FILE_NAME;
    static const std::string DATA_FILE_NAME;

    OplogIndexSegment(configuration_t configuration, logger_t logger, uint32_t oplogId);

    OplogSegmentBetweenTimestampsSearchResult getBetweenTimestamps(uint64_t fromTimestamp, uint64_t toTimestamp);

    OplogSegmentAfterTimestampsSearchResult getByAfterTimestamp(uint64_t timestamp);

    std::vector<OplogIndexSegmentDescriptor> getAll();

    std::result<std::vector<uint8_t>> getDataByDescriptorBytes(OplogIndexSegmentDescriptor descriptor);

    void updateCorruptedSegment(const std::vector<uint8_t>& uncorruptedBytes, uint32_t uncompressedSize, uint64_t indexSegmentDescriptorPtr);

    void save(const std::vector<uint8_t>& toSave);

    void clearAll();

private:
    void initializeFiles();

    OplogSegmentBinarySearchResult binarySearchByTimestamp(uint64_t timestamp);
};

using oplogIndexSegment_t = std::shared_ptr<OplogIndexSegment>;
