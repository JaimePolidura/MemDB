#pragma once

#include "shared.h"

#include "messages/request/Request.h"
#include "utils/strings/SimpleString.h"

using timestampsByKeysMap_t = std::map<SimpleString<memDbDataLength_t>, uint64_t>;

class SingleThreadedLogCompacter {
public:
    auto compact(const std::vector<OperationBody>& uncompacted,
                 const std::vector<OperationBody>& compacted = {},
                 const timestampsByKeysMap_t& timestampsByKeys = {}) -> std::vector<OperationBody>;
};