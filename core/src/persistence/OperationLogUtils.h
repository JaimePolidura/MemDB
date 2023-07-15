#pragma once

#include "shared.h"
#include "messages/request/Request.h"

class OperationLogUtils {
public:
    static std::vector<SimpleString<memDbDataLength_t>> getUniqueKeys(const std::vector<OperationBody>& operationLogs);
};