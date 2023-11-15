#include "operators/DelayedOperationsBuffer.h"

DelayedOperationsBuffer::DelayedOperationsBuffer(configuration_t configuration):
    diskBuffer(configuration->get(ConfigurationKeys::DATA_PATH), "pending-requests-to-handle") {}


void DelayedOperationsBuffer::add(const Request& request){
    this->diskBuffer.add(request);
}

BackedDiskBufferIterator DelayedOperationsBuffer::iterator() {
    return this->diskBuffer.iterator();
}
