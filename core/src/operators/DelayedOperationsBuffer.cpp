#include "operators/DelayedOperationsBuffer.h"

DelayedOperationsBuffer::DelayedOperationsBuffer(configuration_t configuration):
    diskBuffer(std::make_shared<BackedDiskRequestBuffer>(configuration->get(ConfigurationKeys::DATA_PATH), "pending-requests-to-handle")) {}

void DelayedOperationsBuffer::add(const Request& request){
    this->diskBuffer->add(request);
}

iterator_t<Request> DelayedOperationsBuffer::iterator() {
    return this->diskBuffer->iterator();
}
