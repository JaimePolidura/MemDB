#pragma once

#include <vector>
#include <map>
#include <set>
#include <unordered_set>
#include <iostream>

#include "messages/request/Request.h"

class SingleThreadedLogCompacter {
public:
    auto compact(const std::vector<OperationBody>& uncompacted) -> std::vector<OperationBody> {
        if(uncompacted.empty())
            return uncompacted;

        std::unordered_set<SimpleString<defaultMemDbLength_t>, SimpleStringHash<defaultMemDbLength_t>, SimpleStringEqual<defaultMemDbLength_t>> seenOperationKeys;
        std::vector<OperationBody> compacted{};

        for(int i = uncompacted.size() - 1; i >= 0; i--) {
            auto actualOperation = uncompacted[i];
            auto actualOperationKey = * actualOperation.args->begin();

            if(seenOperationKeys.contains(actualOperationKey)){
                continue;
            }

            seenOperationKeys.insert(actualOperationKey);
            compacted.push_back(actualOperation);
        }

        return compacted;
    }
};