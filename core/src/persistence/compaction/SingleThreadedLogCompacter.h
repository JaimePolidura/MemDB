#pragma once

#include "shared.h"

#include "messages/request/Request.h"
#include "utils/strings/SimpleString.h"

class SingleThreadedLogCompacter {
public:
    auto compact(const std::vector<OperationBody>& uncompacted,
                 const std::vector<OperationBody>& compacted = {},
                 const setSimpleString_t& seenOperationKeys = {}) -> std::vector<OperationBody>;
};