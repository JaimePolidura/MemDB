#pragma once

#include "shared.h"
#include "messages/request/Request.h"

class OperationLogUtils {
public:
    static std::vector<SimpleString<memDbDataLength_t>> getUniqueKeys(const std::vector<OperationBody>& operationLogs) {
        setSimpleString_t keysAlreadySeen{};

        for (const OperationBody& operationLog: operationLogs) {
            arg_t key = operationLog.getArg(0);

            if(!keysAlreadySeen.contains(key)) {
                keysAlreadySeen.insert(key);
            }
        }

        return std::vector<SimpleString<memDbDataLength_t>>(keysAlreadySeen.begin(), keysAlreadySeen.end());
    }
};