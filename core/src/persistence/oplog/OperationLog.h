#pragma once

#include "messages/request/Request.h"
#include "config/Configuration.h"

struct OperationLogQueryOptions {
    int operationLogId;
};

class OperationLog {
protected:
    configuration_t configuration;

public:
    OperationLog(configuration_t configuration): configuration(configuration) {}

    virtual void add(const OperationBody& operation) = 0;

    virtual void replaceAll(std::vector<OperationBody> toReplace, const OperationLogQueryOptions options = {}) = 0;

    virtual std::vector<OperationBody> getAfterTimestamp(uint64_t timestamp, const OperationLogQueryOptions options = {}) = 0;

    virtual std::vector<OperationBody> getAllFromDisk(const OperationLogQueryOptions options = {}) = 0;
};

using operationLog_t = std::shared_ptr<OperationLog>;