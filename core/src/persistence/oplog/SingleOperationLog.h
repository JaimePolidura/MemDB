#pragma once

#include "shared.h"

#include "messages/request/Request.h"

#include "persistence/compaction/OperationLogCompacter.h"
#include "persistence/OperationLogDiskLoader.h"
#include "persistence/OperationLogBuffer.h"
#include "persistence/OperationLogDiskWriter.h"
#include "persistence/oplog/OperationLog.h"

#include "config/Configuration.h"
#include "utils/Utils.h"

class SingleOperationLog : public OperationLog {
private:
    OperationsLogDiskWriter operationsLogDiskWriter;
    OperationLogDiskLoader operationLogDiskLoader;
    OperationLogCompacter compacter{};

    operationLogBuffer_t operationLogBuffer;

public:
    SingleOperationLog() = default;

    SingleOperationLog(configuration_t configuration, const std::string& fileName);

    void addAll(const std::vector<OperationBody>& operations, const OperationLogOptions options) override;

    void add(const OperationBody& operation, const OperationLogOptions options) override;

    bool hasOplogFile(const OperationLogOptions options) override;

    std::vector<OperationBody> clear(const OperationLogOptions options) override;

    std::vector<OperationBody> getAfterTimestamp(uint64_t since, const OperationLogOptions options) override;

    std::vector<OperationBody> get(const OperationLogOptions option) override;

    uint32_t getNumberOplogFiles() override;
    
    void flush() override;

private:
    std::vector<OperationBody> filterIfTimestampAfterThan(const std::vector<OperationBody>& operations, uint64_t timestampSince);

    void flushToDisk(const std::vector<OperationBody>& operationsInBuffer);
};

using singleOperationLog_t = std::shared_ptr<SingleOperationLog>;