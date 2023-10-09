#pragma once

#include "shared.h"

class MultiResponseReceiverIterator {
private:
    uint64_t nTotalFragments;
    uint64_t actualFragment;

    std::function<std::vector<uint8_t>(uint64_t)> nextFragment;

public:
    MultiResponseReceiverIterator(uint64_t nTotalFragments, std::function<std::vector<uint8_t>(uint64_t)> nextFragment);

    bool hasNext();

    std::vector<uint8_t> next();

    static MultiResponseReceiverIterator emtpy();
};

