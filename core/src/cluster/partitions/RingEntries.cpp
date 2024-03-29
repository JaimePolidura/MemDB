#include "cluster/partitions/RingEntries.h"

void RingEntries::add(RingEntry ringEntryToAdd) {
    if(this->indexByNodeId.contains(ringEntryToAdd.nodeId)){
        return;
    }

    uint32_t size = this->indexByNodeId.size();
    RingEntryNode * newRingEntry = new RingEntryNode(ringEntryToAdd);
    this->indexByNodeId[ringEntryToAdd.nodeId] = newRingEntry;

    if(size == 0){
        this->head = newRingEntry;
        return;
    }
    if(size == 1) {
        if(this->head->entry.ringPosition > ringEntryToAdd.ringPosition){
            auto prevHeadNode = this->head;

            this->head = newRingEntry;
            prevHeadNode->next = newRingEntry;
            prevHeadNode->back = newRingEntry;
            this->head->next = prevHeadNode;
            this->head->back = prevHeadNode;
        } else {
            this->head->next = newRingEntry;
            this->head->back = newRingEntry;
            newRingEntry->next = this->head;
            newRingEntry->back = this->head;
        }

        return;
    }

    uint32_t iterations = 0;
    auto actual = this->head;
    auto nextToActual = this->head->next;
    while(size >= iterations) {
        if(actual->entry.ringPosition < ringEntryToAdd.ringPosition &&
           nextToActual->entry.ringPosition > ringEntryToAdd.ringPosition){

            actual->next = newRingEntry;
            newRingEntry->back = actual;

            nextToActual->back = newRingEntry;
            newRingEntry->next = nextToActual;

            return;
        }

        actual = nextToActual;
        nextToActual = actual->next;

        iterations++;
    }

    //New ring entry has the larges ringPosition so it will be added before HEAD
    auto oldBackToHead = this->head->back;
    this->head->back = newRingEntry;
    oldBackToHead->next = newRingEntry;
    newRingEntry->next = this->head;
    newRingEntry->back = oldBackToHead;
}

std::vector<RingEntry> RingEntries::getAll() {
    std::vector<RingEntry> ringEntriesToReturn{};
    
    for(const auto[key, value] : this->indexByNodeId){
        ringEntriesToReturn.push_back(value->entry);
    }
    
    return ringEntriesToReturn;
}

void RingEntries::deleteByNodeId(memdbNodeId_t nodeId) {
    RingEntryNode * ringEntryDeleted = this->indexByNodeId.at(nodeId);
    ringEntryDeleted->back->next = ringEntryDeleted->next;
    ringEntryDeleted->next->back = ringEntryDeleted->back;

    this->indexByNodeId.erase(nodeId);
}

std::vector<RingEntry> RingEntries::getNeighborsClockwise(memdbNodeId_t nodeId, uint32_t numberNeighbors) {
    std::vector<RingEntry> neighbors{};
    RingEntryNode * initialNodePtr = this->indexByNodeId.at(nodeId);
    RingEntryNode * startedNodePtr = initialNodePtr;

    for(int i = 0; i < numberNeighbors; i++){
        if(i > 0 && initialNodePtr->next == startedNodePtr){
            break;
        }
        if(initialNodePtr->next == nullptr) {
            break;
        }
        initialNodePtr = initialNodePtr->next;
        neighbors.push_back(initialNodePtr->entry);
    }

    return neighbors;
}

std::vector<RingEntry> RingEntries::getNeighborsCounterClockwise(memdbNodeId_t nodeId, uint32_t numberNeighbors) {
    std::vector<RingEntry> neighbors{};
    RingEntryNode * initialNodePtr = this->indexByNodeId.at(nodeId);
    RingEntryNode * startedNodePtr = initialNodePtr;

    for(int i = 0; i < numberNeighbors; i++){
        if(i > 0 && initialNodePtr->back == startedNodePtr){
            break;
        }
        if(initialNodePtr->next == nullptr) {
            break;
        }

        initialNodePtr = initialNodePtr->back;
        neighbors.push_back(initialNodePtr->entry);
    }

    return neighbors;
}

std::optional<RingEntry> RingEntries::getNeighborCounterClockwise(memdbNodeId_t nodeId) {
    if(this->indexByNodeId.contains(nodeId)) {
        RingEntryNode * ringEntryByNodeId = this->indexByNodeId.at(nodeId);

        return ringEntryByNodeId->back != nullptr ?
            std::optional<RingEntry>(ringEntryByNodeId->back->entry) :
            std::nullopt;
    } else {
        return std::nullopt;
    }
}

std::optional<RingEntry> RingEntries::getNeighborClockwise(memdbNodeId_t nodeId) {
    if(this->indexByNodeId.contains(nodeId)) {
        RingEntryNode * ringEntryByNodeId = this->indexByNodeId.at(nodeId);

        return ringEntryByNodeId->next != nullptr ?
            std::optional<RingEntry>(ringEntryByNodeId->next->entry) :
            std::nullopt;
    } else {
        return std::nullopt;
    }
}

RingEntry RingEntries::getRingEntryBelongsToPosition(uint32_t ringPosition) {
    RingEntryNode * actualPointer = this->head;
    RingEntryNode * nextToActualPointer = actualPointer;
    uint32_t size = this->indexByNodeId.size();
    uint32_t iterations = 0;

    while(size >= iterations) {
        actualPointer = nextToActualPointer;
        nextToActualPointer = actualPointer->next;

        if(nextToActualPointer == nullptr) {
            return actualPointer->entry;
        }
        if(actualPointer->entry.ringPosition <= ringPosition && nextToActualPointer->entry.ringPosition > ringPosition){
            return actualPointer->entry;
        }
        if(nextToActualPointer == this->head){
            return actualPointer->entry;
        }

        iterations++;
    }

    return this->head->entry;
}

// nodeA --> nodeB -> positive
// nodeB --> nodeA -> negative
int RingEntries::getDistance(memdbNodeId_t nodeA, memdbNodeId_t nodeB) {
    uint32_t size = this->indexByNodeId.size();
    int iterations = 0;

    auto pointerNodeA = this->indexByNodeId.at(nodeA);
    auto pointerNodeB = this->indexByNodeId.at(nodeB);

    auto actualPointerA = pointerNodeA;
    auto actualPointerB = pointerNodeB;

    while(size >= iterations) {
        if(actualPointerA->getNodeId() == nodeB){
            return iterations;
        }
        if(actualPointerB->getNodeId() == nodeA){
            return - iterations;
        }

        actualPointerA = actualPointerA->next;
        actualPointerB = actualPointerB->next;

        iterations++;
    }

    return actualPointerB->getNodeId() == nodeA ? iterations * -1 : iterations;
}

// nodeA <-- (memdb_thread_pool_counter clockwise) nodeB
uint32_t RingEntries::getDistanceCounterClockwise(memdbNodeId_t nodeA, memdbNodeId_t nodeB) {
    int distance = this->getDistance(nodeA, nodeB);

    return distance > 0 ?
           this->indexByNodeId.size() - distance :
           std::abs(distance);
}

// nodeA --> (clockwise) nodeB
uint32_t RingEntries::getDistanceClockwise(memdbNodeId_t nodeA, memdbNodeId_t nodeB) {
    int distance = this->getDistance(nodeA, nodeB);

    return distance < 0 ?
           this->indexByNodeId.size() - std::abs(distance) :
           distance;
}

RingEntry RingEntries::getByNodeId(memdbNodeId_t nodeId) {
    return this->indexByNodeId.at(nodeId)->entry;
}

RingEntries RingEntries::fromEntries(std::vector<RingEntry> entries) {
    std::sort(entries.begin(), entries.end(), [](RingEntry a, RingEntry b) { return b.ringPosition > a.ringPosition; });

    std::map<memdbNodeId_t, RingEntryNode *> indexByNodeId{};
    RingEntryNode * head = nullptr;
    RingEntryNode * prev = nullptr;

    for(int i = 0; i < entries.size(); i++){
        auto * ringEntryNode = new RingEntryNode(entries[i]);

        if(i == 0){ //First iteration
            head = ringEntryNode;
        }else if(i == entries.size() - 1){ //Last iteration
            ringEntryNode->back = prev;
            prev->next = ringEntryNode;
            head->back = ringEntryNode;
            ringEntryNode->next = head;
        }else{ //Middle iteration
            ringEntryNode->back = prev;
            prev->next = ringEntryNode;
        }

        indexByNodeId.insert({ringEntryNode->entry.nodeId, ringEntryNode});

        prev = ringEntryNode;
    }

    return RingEntries{head, std::move(indexByNodeId)};
}
