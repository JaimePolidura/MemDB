#pragma once

#include "shared.h"
#include "utils/strings/SimpleString.h"
#include "utils/Iterator.h"

using nextFragment_t = std::function<std::vector<uint8_t>(uint64_t multiResponseId, uint64_t fragmentId)>;

class MultiResponseReceiverIterator : public Iterator<std::vector<uint8_t>> {
private:
    uint64_t multiResponseId;
    uint64_t nTotalFragments;
    uint64_t actualFragment;

    nextFragment_t nextFragment;

public:
    using multiResponseReceiverIterator_t = std::shared_ptr<MultiResponseReceiverIterator>;

    MultiResponseReceiverIterator(uint64_t multiResponseId, uint64_t nTotalFragments, nextFragment_t nextFragment);

    bool hasNext() override;

    std::vector<uint8_t> next() override;

    uint64_t totalSize() override;

    static multiResponseReceiverIterator_t emtpy();
};

using multiResponseReceiverIterator_t = std::shared_ptr<MultiResponseReceiverIterator>;