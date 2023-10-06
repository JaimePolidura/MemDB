#pragma once

#include "shared.h"

#include "messages/request/Request.h"

#include "persistence/compaction/OperationLogCompacter.h"
#include "persistence/OperationLogDiskLoader.h"
#include "persistence/OperationLogDiskWriter.h"
#include "persistence/buffer/OperationLogBuffer.h"
#include "persistence/oplog/OperationLog.h"

#include "persistence/v2/intermediate/IntermediateOplog.h"
#include "persistence/v2/index/OplogIndexSegment.h"

#include "config/Configuration.h"
#include "utils/Utils.h"

class SingleOperationLog : public OperationLog {
private:
    const std::string memdbBasePath;
    const std::string partitionPath;
    const uint32_t oplogId{};

    oplogIndexSegment_t oplogIndexSegment_t;
    intermediateOplog_t intermediateOplog;

public:
    SingleOperationLog() = default;

    SingleOperationLog(configuration_t configuration, uint32_t oplogId);

    void addAll(const std::vector<OperationBody>& operations, const OperationLogOptions options) override; //Implemented

    void add(const OperationBody& operation, const OperationLogOptions options) override; //Implemented

    bool hasOplogFile(const OperationLogOptions options) override; //Implemented

    std::vector<OperationBody> clear(const OperationLogOptions options) override;

    std::vector<OperationBody> getAfterTimestamp(uint64_t since, const OperationLogOptions options) override;

    std::vector<OperationBody> get(const OperationLogOptions option) override;

    uint32_t getNumberOplogFiles() override; //Implemented

private:
    void flushFromIntermediate(const std::vector<uint8_t>& toFlush);

    void initializeFiles();  //Implemented
};

using singleOperationLog_t = std::shared_ptr<SingleOperationLog>;