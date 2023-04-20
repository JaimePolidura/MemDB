#pragma once

#include "shared.h"

#include "messages/request/Request.h"

using alreadySennKeys_t = std::unordered_set<SimpleString<memDbDataLength_t>, SimpleStringHash<memDbDataLength_t>, SimpleStringEqual<memDbDataLength_t>>;

class SingleThreadedLogCompacter {
public:
    auto compact(const std::vector<OperationBody>& uncompacted,
                 const std::vector<OperationBody>& compacted = {},
                 const alreadySennKeys_t& seenOperationKeys = {}) -> std::vector<OperationBody> {
        if(uncompacted.empty()) {
            return uncompacted;
        }

        for(int i = uncompacted.size() - 1; i >= 0; i--) {
            auto actualOperation = uncompacted[i];
            auto actualOperationKey = * actualOperation.args->begin();

            if(seenOperationKeys.contains(actualOperationKey)){
                continue;
            }

            const_cast<alreadySennKeys_t&>(seenOperationKeys).insert(actualOperationKey);
            const_cast<std::vector<OperationBody>&>(compacted).push_back(actualOperation);
        }

        return compacted;
    }
};