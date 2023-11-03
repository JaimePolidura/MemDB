#pragma once

#include "messages/request/Request.h"
#include "config/Configuration.h"
#include "persistence/OplogIterator.h"
#include "utils/std/Result.h"

using bytesDiskIterator_t = iterator_t<std::result<std::vector<uint8_t>>>;

struct OperationLogOptions {
    memdbOplogId_t operationLogId;
    bool compressed;
};

class OperationLog {
protected:
    configuration_t configuration;

public:
    explicit OperationLog(configuration_t configuration): configuration(configuration) {}

    OperationLog() = default;

    virtual void add(memdbOplogId_t oplogId, const OperationBody& operation) = 0;

    virtual void addAll(memdbOplogId_t oplogId, const std::vector<OperationBody>& toAddAll) = 0;

    virtual bool hasOplogFile(memdbOplogId_t oplogId) = 0;

    virtual void clear(memdbOplogId_t oplogId) = 0;

    virtual void updateCorrupted(const std::vector<uint8_t>& uncorrupted, uint32_t uncompressedSize, uint64_t ptr, memdbOplogId_t oplogId) = 0;

    virtual bytesDiskIterator_t getBetweenTimestamps(uint64_t fromTimestamp, uint64_t toTimestamp, const OperationLogOptions options) = 0;

    virtual bytesDiskIterator_t getAfterTimestamp(uint64_t timestamp, const OperationLogOptions options) = 0;

    virtual bytesDiskIterator_t getAll(const OperationLogOptions options) = 0;

    virtual uint32_t getNumberOplogFiles() = 0;
};

using operationLog_t = std::shared_ptr<OperationLog>;