#pragma once

#include "messages/request/Request.h"
#include "config/Configuration.h"

struct OperationLogOptions {
    int operationLogId;
    bool dontUseBuffer;
};

class OperationLog {
protected:
    configuration_t configuration;

public:
    OperationLog(configuration_t configuration): configuration(configuration) {}

    virtual void add(const OperationBody& operation, const OperationLogOptions options = {}) = 0;

    virtual void addAll(const std::vector<OperationBody>& toAddAll, const OperationLogOptions options = {}) = 0;

    virtual std::vector<OperationBody> clear(const OperationLogOptions options = {}) = 0;

    virtual void replaceAll(const std::vector<OperationBody>& toReplace, const OperationLogOptions options = {}) = 0;

    virtual std::vector<OperationBody> getAfterTimestamp(uint64_t timestamp, const OperationLogOptions options = {}) = 0;

    virtual std::vector<OperationBody> getAllFromDisk(const OperationLogOptions options = {}) = 0;
};

using operationLog_t = std::shared_ptr<OperationLog>;