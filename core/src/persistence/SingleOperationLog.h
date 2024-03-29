#pragma once

#include "shared.h"

#include "messages/request/Request.h"

#include "persistence/segments/OplogIndexSegment.h"
#include "persistence/buffer/OperationLogBuffer.h"
#include "persistence/OperationLog.h"

#include "persistence/intermediate/IntermediateOplog.h"
#include "persistence/segments/OplogIndexSegmentDescriptor.h"

#include "config/Configuration.h"
#include "logging/Logger.h"

class SingleOperationLog : public OperationLog {
public:
    SingleOperationLog(configuration_t configuration, logger_t logger);

    void add(memdbOplogId_t oplogId, const OperationBody& operation) override;

    void addAll(memdbOplogId_t oplogId, const std::vector<OperationBody>& operations) override;

    bool hasOplogFile(memdbOplogId_t oplogId) override;

    void clear(memdbOplogId_t oplogId) override;

    void updateCorrupted(const std::vector<uint8_t>& uncorrupted, uint32_t uncompressedSize, uint64_t ptr, memdbOplogId_t oplogId) override;

    bytesDiskIterator_t getBetweenTimestamps(uint64_t fromTimestamp, uint64_t toTimestamp, const OperationLogOptions options) override;

    bytesDiskIterator_t getAfterTimestamp(uint64_t since, const OperationLogOptions options) override;

    bytesDiskIterator_t getAll(const OperationLogOptions options) override;

    uint32_t getNumberOplogFiles() override;

private:
    friend class MultipleOperationLog;
    friend class MemDbCreator;

    std::string memdbBasePath;
    std::string partitionPath;

    operationLogBuffer_t operationLogBuffer;
    oplogIndexSegment_t oplogIndexSegment;
    intermediateOplog_t intermediateOplog;

    void initialize(uint32_t oplogId);

    void initializeFiles();

    std::result<std::vector<uint8_t>> readBytesByIndexSegmentDescriptor(OplogIndexSegmentDescriptor descriptor);
};

using singleOperationLog_t = std::shared_ptr<SingleOperationLog>;