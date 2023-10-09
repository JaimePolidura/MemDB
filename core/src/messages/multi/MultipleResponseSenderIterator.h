#pragma once

#include "shared.h"

class MultipleResponseSenderIterator {
public:
    virtual bool hasNext() = 0;
    virtual std::vector<uint8_t> next() = 0;
    virtual uint64_t size() = 0;
};

using multipleResponseSenderIterator_t = std::shared_ptr<MultipleResponseSenderIterator>;