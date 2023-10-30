#pragma once

#include "shared.h"

#include "messages/request/Request.h"

#include "persistence/compaction/OperationLogCompacter.h"
#include "persistence/segments/OplogIndexSegment.h"
#include "persistence/buffer/OperationLogBuffer.h"
#include "persistence/OperationLog.h"

#include "persistence/intermediate/IntermediateOplog.h"
#include "persistence/segments/OplogIndexSegment.h"
#include "persistence/OplogIterator.h"
#include "persistence/segments/OplogIndexSegmentDescriptor.h"

#include "config/Configuration.h"
#include "utils/Utils.h"
#include "logging/Logger.h"

class SingleOperationLog : public OperationLog {
private:
    const std::string memdbBasePath;
    const std::string partitionPath;

    operationLogBuffer_t operationLogBuffer;
    oplogIndexSegment_t oplogIndexSegment;
    intermediateOplog_t intermediateOplog;
    logger_t logger;

public:
    SingleOperationLog(configuration_t configuration, uint32_t oplogId, logger_t logger);

    void addAll(const std::vector<OperationBody>& operations, const OperationLogOptions options) override;

    void add(const OperationBody& operation, const OperationLogOptions options) override;

    bool hasOplogFile(const OperationLogOptions options) override;

    void clear(const OperationLogOptions options) override;

    void updateCorrupted(const std::vector<uint8_t>& uncorrupted, uint32_t uncompressedSize, uint64_t ptr, const OperationLogOptions options) override;

    bytesDiskIterator_t getBetweenTimestamps(uint64_t fromTimestamp, uint64_t toTimestamp, const OperationLogOptions options) override;

    bytesDiskIterator_t getAfterTimestamp(uint64_t since, const OperationLogOptions options) override;

    bytesDiskIterator_t getAll(const OperationLogOptions option) override;

    uint32_t getNumberOplogFiles() override;

private:
    void initializeFiles();

    std::result<std::vector<uint8_t>> readBytesByIndexSegmentDescriptor(OplogIndexSegmentDescriptor descriptor);
};

using singleOperationLog_t = std::shared_ptr<SingleOperationLog>;