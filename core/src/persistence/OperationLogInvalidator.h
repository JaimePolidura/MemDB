#pragma once

#include "persistence/utils/OperationLogUtils.h"
#include "messages/request/Request.h"
#include "shared.h"

class OperationLogInvalidator {
public:
    std::vector<OperationBody> getInvalidationOperations(const std::vector<OperationBody>& toInvalidate);

private:
    OperationBody createDeleteKeyOperation(const SimpleString<memDbDataLength_t>& key);
};