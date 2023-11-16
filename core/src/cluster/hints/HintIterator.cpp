#include "HintIterator.h"

HintIterator::HintIterator(backedDiskBufferIterator_t diskBufferIterator, std::function<void()> onStopIterating):
    diskBufferIterator(diskBufferIterator), onStopIterating(onStopIterating) {}

Request HintIterator::next() {
    return this->diskBufferIterator->next();
}

uint64_t HintIterator::totalSize() {
    return this->diskBufferIterator->totalSize();
}

bool HintIterator::hasNext() {
    bool bufferIteratorHasNext = this->diskBufferIterator->hasNext();
    if(!bufferIteratorHasNext) {
        this->onStopIterating();
    }

    return bufferIteratorHasNext;
}

