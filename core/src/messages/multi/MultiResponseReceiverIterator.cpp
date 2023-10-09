#include "MultiResponseReceiverIterator.h"


MultiResponseReceiverIterator::MultiResponseReceiverIterator(uint64_t nTotalFragments, std::function<std::vector<uint8_t>(uint64_t)> nextFragment):
        nextFragment(nextFragment),
        actualFragment(0),
        nTotalFragments(nTotalFragments) {}

bool MultiResponseReceiverIterator::hasNext() {
    return nTotalFragments > actualFragment;
}

std::vector<uint8_t> MultiResponseReceiverIterator::next() {
    return this->nextFragment(this->actualFragment++);
}

MultiResponseReceiverIterator MultiResponseReceiverIterator::emtpy() {
    return MultiResponseReceiverIterator{0, nullptr};
}