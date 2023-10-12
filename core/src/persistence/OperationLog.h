#pragma once

#include "messages/request/Request.h"
#include "config/Configuration.h"
#include "persistence/OplogSegmentIterator.h"

struct OperationLogOptions {
    uint32_t operationLogId;
};

class OperationLog {
protected:
    configuration_t configuration;

public:
    explicit OperationLog(configuration_t configuration): configuration(configuration) {}

    OperationLog() = default;

    virtual void add(const OperationBody& operation, const OperationLogOptions options = {}) = 0;

    virtual void addAll(const std::vector<OperationBody>& toAddAll, const OperationLogOptions options = {}) = 0;

    virtual bool hasOplogFile(const OperationLogOptions options = {}) = 0;

    virtual void clear(const OperationLogOptions options = {}) = 0;

    virtual oplogSegmentIterator_t getAfterTimestamp(uint64_t timestamp, const OperationLogOptions options = {}) = 0;

    virtual oplogSegmentIterator_t getAll(const OperationLogOptions options = {}) = 0;

    virtual uint32_t getNumberOplogFiles() = 0;
};

using operationLog_t = std::shared_ptr<OperationLog>;