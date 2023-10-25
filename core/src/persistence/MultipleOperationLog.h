#pragma once

#include "OperationLog.h"
#include "SingleOperationLog.h"
#include "logging/Logger.h"

class MultipleOperationLog : public OperationLog {
private:
    std::vector<singleOperationLog_t> operationLogs;

    std::function<int(const OperationBody&)> oplogResolver;

    logger_t logger;

public:
    MultipleOperationLog(configuration_t configuration, std::function<int(const OperationBody&)> oplogResolver,
                         std::function<std::string(int)> oplogFileNameResolver, uint32_t numberOplogs, logger_t loggerCons);

    void add(const OperationBody& operation, const OperationLogOptions options = {}) override;

    void addAll(const std::vector<OperationBody>& operations, const OperationLogOptions options = {}) override;

    bool hasOplogFile(const OperationLogOptions options = {}) override;

    void clear(const OperationLogOptions options = {}) override;

    iterator_t<std::vector<uint8_t>> getAfterTimestamp(uint64_t timestamp, OperationLogOptions options) override;

    iterator_t<std::vector<uint8_t>> getAll(const OperationLogOptions options = {}) override;

    uint32_t getNumberOplogFiles() override;

private:
    void initializeOplogs(int numberOplogs, std::function<std::string(int)> oplogFileNameResolver);
};
