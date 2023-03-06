#pragma once

#include <vector>
#include <map>
#include <set>
#include <iostream>

#include "messages/request/Request.h"

class OperationLogCompacter {
public:
    auto compact(const std::vector<OperationBody> uncompacted) -> std::vector<OperationBody> {
        if(uncompacted.empty())
            return uncompacted;

        std::set<SimpleString<defaultMemDbLength_t>> seenOperationKeys;
        std::vector<OperationBody> compacted{};

        for(int i = uncompacted.size() - 1; i >= 0; i--) {
            auto actualOperation = uncompacted.at(i);
            auto actualOperationKey = actualOperation.args->at(0);

            if(seenOperationKeys.contains(actualOperationKey))
                continue;

            seenOperationKeys.insert(actualOperationKey);
            compacted.push_back(actualOperation);
        }

        return compacted;
    }
};