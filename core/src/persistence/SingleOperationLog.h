#pragma once

#include "shared.h"

#include "messages/request/Request.h"

#include "persistence/compaction/OperationLogCompacter.h"
#include "persistence/buffer/OperationLogBuffer.h"
#include "OperationLog.h"

#include "persistence/intermediate/IntermediateOplog.h"
#include "persistence/segments/OplogIndexSegment.h"
#include "persistence/OplogSegmentIterator.h"
#include "persistence/segments/OplogIndexSegmentDescriptor.h"

#include "config/Configuration.h"
#include "utils/Utils.h"

class SingleOperationLog : public OperationLog {
private:
    const std::string memdbBasePath;
    const std::string partitionPath;

    operationLogBuffer_t operationLogBuffer;
    oplogIndexSegment_t oplogIndexSegment_t;
    intermediateOplog_t intermediateOplog;

public:
    SingleOperationLog(configuration_t configuration, uint32_t oplogId);

    void addAll(const std::vector<OperationBody>& operations, const OperationLogOptions options) override; //Implemented

    void add(const OperationBody& operation, const OperationLogOptions options) override; //Implemented

    bool hasOplogFile(const OperationLogOptions options) override; //Implemented

    std::vector<OperationBody> clear(const OperationLogOptions options) override;

    OplogSegmentIterator getAfterTimestamp(uint64_t since, const OperationLogOptions options) override; //Implemented
    
    OplogSegmentIterator get(const OperationLogOptions option) override; //Implemented

    uint32_t getNumberOplogFiles() override; //Implemented

private:
    void initializeFiles();  //Implemented
};

using singleOperationLog_t = std::shared_ptr<SingleOperationLog>;