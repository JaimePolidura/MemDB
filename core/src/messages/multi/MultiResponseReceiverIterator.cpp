#include "MultiResponseReceiverIterator.h"


MultiResponseReceiverIterator::MultiResponseReceiverIterator(uint64_t multiResponseId, uint64_t nTotalFragments, nextFragment_t nextFragment):
        multiResponseId(multiResponseId),
        nextFragment(nextFragment),
        actualFragment(0),
        nTotalFragments(nTotalFragments) {}

bool MultiResponseReceiverIterator::hasNext() {
    return nTotalFragments > actualFragment;
}

std::vector<uint8_t> MultiResponseReceiverIterator::next() {
    return this->nextFragment(this->multiResponseId, this->actualFragment++);
}

uint64_t MultiResponseReceiverIterator::size() {
    return this->nTotalFragments - this->actualFragment;
}

multiResponseReceiverIterator_t MultiResponseReceiverIterator::emtpy() {
    return std::make_shared<MultiResponseReceiverIterator>(0, 0, nullptr);
}