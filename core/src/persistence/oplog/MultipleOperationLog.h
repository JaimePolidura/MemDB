#pragma once

#include "persistence/oplog/OperationLog.h"
#include "persistence/oplog/SingleOperationLog.h"

class MultipleOperationLog : public OperationLog {
private:
    std::vector<singleOperationLog_t> operationLogs;

    std::function<int(const OperationBody&)> oplogResolver;

public:
    MultipleOperationLog(configuration_t configuration, std::function<int(const OperationBody&)> oplogResolver,
                         std::function<std::string(int)> oplogFileNameResolver, uint32_t numberOplogs);

    void addAll(const std::vector<OperationBody>& operations, const OperationLogOptions options = {}) override;

    void add(const OperationBody& operation, const OperationLogOptions options = {}) override;

    bool hasOplogFile(const OperationLogOptions options = {}) override;

    std::vector<OperationBody> clear(const OperationLogOptions options = {}) override;

    std::vector<OperationBody> getAfterTimestamp(uint64_t timestamp, OperationLogOptions options) override;

    std::vector<OperationBody> get(const OperationLogOptions options = {}) override;

    uint32_t getNumberOplogFiles() override;

private:
    void initializeOplogs(int numberOplogs, std::function<std::string(int)> oplogFileNameResolver);
};
