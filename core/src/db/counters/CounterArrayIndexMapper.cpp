#include "CounterArrayIndexMapper.h"

int CounterArrayIndexMapper::getIndex(memdbNodeId_t nodeId) {
    if(this->indexByNodeId.contains(nodeId)) {
        return this->indexByNodeId.at(nodeId);
    } else {
        int newIndex = ++this->lastIndexAssigned;
        this->indexByNodeId[nodeId] = newIndex;
        return newIndex;
    }
}
