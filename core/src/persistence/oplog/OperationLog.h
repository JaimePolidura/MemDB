#pragma once

#include "messages/request/Request.h"
#include "config/Configuration.h"

struct OperationLogOptions {
    uint32_t operationLogId;
    bool dontUseBuffer;

    bool operator==(const OperationLogOptions& other) const {
        return operationLogId == other.operationLogId && dontUseBuffer == other.dontUseBuffer;
    }
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

    virtual std::vector<OperationBody> clear(const OperationLogOptions options = {}) = 0;

    virtual std::vector<OperationBody> getAfterTimestamp(uint64_t timestamp, const OperationLogOptions options = {}) = 0;

    virtual std::vector<OperationBody> get(const OperationLogOptions options = {}) = 0;

    virtual uint32_t getNumberOplogFiles() = 0;
};

using operationLog_t = std::shared_ptr<OperationLog>;