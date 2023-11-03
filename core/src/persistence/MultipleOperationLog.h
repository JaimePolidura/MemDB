#pragma once

#include "OperationLog.h"
#include "SingleOperationLog.h"
#include "logging/Logger.h"

class MultipleOperationLog : public OperationLog {
private:
    std::vector<singleOperationLog_t> operationLogs;

    logger_t logger;

public:
    MultipleOperationLog(configuration_t configuration, std::function<std::string(int)> oplogFileNameResolver,
                         uint32_t numberOplogs, logger_t loggerCons);

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
    void initializeOplogs(int numberOplogs, std::function<std::string(int)> oplogFileNameResolver);
};
