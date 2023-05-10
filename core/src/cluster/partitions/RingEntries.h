#pragma once

#include "cluster/partitions/RingEntry.h"
#include "cluster/Node.h"

struct RingEntryNode {
    RingEntry entry;

    RingEntryNode * next;
    RingEntryNode * back;

    RingEntryNode() = default;

    RingEntryNode(RingEntry entry): entry(entry) {}

    memdbNodeId_t getNodeId() {
        return this->entry.nodeId;
    }
};

class RingEntries {
private:
    RingEntryNode * head;
    std::map<memdbNodeId_t, RingEntryNode *> indexByNodeId;

public:
    RingEntries(RingEntryNode * head, const std::map<memdbNodeId_t, RingEntryNode *>& indexByNodeId): head(head), indexByNodeId(std::move(indexByNodeId)) {}

    RingEntries() = default;

    void add(RingEntry ringEntryToAdd) {
        uint32_t size = this->indexByNodeId.size();
        RingEntryNode * newRignEntry = new RingEntryNode(ringEntryToAdd);
        this->indexByNodeId[ringEntryToAdd.nodeId] = newRignEntry;

        if(size == 0){
            this->head = newRignEntry;
            return;
        }
        if(size == 1) {
            if(this->head->entry.ringPosition > ringEntryToAdd.ringPosition){
                newRignEntry = this->head;
                this->head = newRignEntry;
            }

            this->head->next = newRignEntry;
            this->head->back = newRignEntry;
            newRignEntry->next = this->head;
            newRignEntry->back = this->head;

            return;
        }

        uint32_t iterations = 0;
        auto actual = this->head;
        auto prevToActual = this->head->back;
        while(size >= iterations) {
            if(actual->entry.ringPosition > ringEntryToAdd.ringPosition &&
                prevToActual->entry.ringPosition < ringEntryToAdd.ringPosition){

                prevToActual->next = newRignEntry;
                newRignEntry->back = prevToActual;
                actual->back = newRignEntry;
                newRignEntry->next = actual;

                return;
            }

            prevToActual = actual;
            actual = actual->next;

            iterations++;
        }
    }

    void deleteByNodeId(memdbNodeId_t nodeId) {
        RingEntryNode * ringEntryDeleted = this->indexByNodeId.at(nodeId);
        ringEntryDeleted->back->next = ringEntryDeleted->next;
        ringEntryDeleted->next->back = ringEntryDeleted->back;
    }

    std::vector<RingEntry> getNeighborsClockwise(memdbNodeId_t nodeId, int numberNeighbors) {
        RingEntryNode * ringEntryOfNode = this->indexByNodeId.at(nodeId);
        std::vector<RingEntry> neighbors(numberNeighbors);

        for(int i = 0; i < numberNeighbors; i++){
            ringEntryOfNode = ringEntryOfNode->next;
            neighbors.push_back(ringEntryOfNode->entry);
        }

        return neighbors;
    }

    RingEntry getNeighborCounterClockwise(memdbNodeId_t nodeId) {
        return this->indexByNodeId.at(nodeId)->back->entry;
    }

    RingEntry getRingEntryBelongsToPosition(uint32_t ringPosition) {
        auto actualPointer = this->head;
        uint32_t size = this->indexByNodeId.size();
        uint32_t iterations = 0;

        while(size >= iterations){
            auto nextToActualPointer = actualPointer->next;

            if(actualPointer->entry.ringPosition >= ringPosition && nextToActualPointer->entry.ringPosition < ringPosition){
                return actualPointer->entry;
            }
            if(nextToActualPointer == this->head){
                return actualPointer->entry;
            }

            iterations++;
        }

        return this->head->entry;
    }

    // nodeA <-- (counter clockwise) nodeB
    uint32_t getDistanceCounterClockwise(memdbNodeId_t nodeA, memdbNodeId_t nodeB) {
        uint32_t size = this->indexByNodeId.size();
        uint32_t iterations = 0;

        auto pointerNodeB = this->indexByNodeId.at(nodeB);
        auto actualPointerB = pointerNodeB;

        while(size >= iterations) {
            if(actualPointerB->getNodeId() == nodeA){
                return iterations;
            }

            pointerNodeB = pointerNodeB->next;
            iterations++;
        }

        return iterations;
    }

    // nodeA --> (clockwise) nodeB
    uint32_t getDistanceClockwise(memdbNodeId_t nodeA, memdbNodeId_t nodeB) {
        uint32_t size = this->indexByNodeId.size();
        uint32_t iterations = 0;

        auto pointerNodeA = this->indexByNodeId.at(nodeA);
        auto actualPointerA = pointerNodeA;

        while(size >= iterations) {
            if(actualPointerA->getNodeId() == nodeB){
                return iterations;
            }

            pointerNodeA = pointerNodeA->next;
            iterations++;
        }

        return iterations;
    }

    uint32_t getDistance(memdbNodeId_t nodeA, memdbNodeId_t nodeB) {
        uint32_t size = this->indexByNodeId.size();
        uint32_t iterations = 0;

        auto pointerNodeA = this->indexByNodeId.at(nodeA);
        auto pointerNodeB = this->indexByNodeId.at(nodeB);

        auto actualPointerA = pointerNodeA;
        auto actualPointerB = pointerNodeB;

        while(size >= iterations) {
            if(actualPointerA->getNodeId() == nodeB || actualPointerB->getNodeId() == nodeA){
                return iterations;
            }

            pointerNodeA = pointerNodeA->next;
            pointerNodeB = pointerNodeB->next;

            iterations++;
        }

        return iterations;
    }

    RingEntry getByNodeId(memdbNodeId_t nodeId) {
        return this->indexByNodeId.at(nodeId)->entry;
    }

    static RingEntries fromEntries(std::vector<RingEntry> entries) {
        std::sort(entries.begin(), entries.end(), [](RingEntry a, RingEntry b) { return a.ringPosition > b.ringPosition; });

        std::map<memdbNodeId_t, RingEntryNode *> indexByNodeId{};
        RingEntryNode * head = nullptr;
        RingEntryNode * prev = nullptr;

        for(int i = 0; i < entries.size(); i++){
            RingEntryNode * ringEntryNode = new RingEntryNode(entries[i]);

            if(i == 0){ //First iteration
                head = ringEntryNode;
            }else if(i == entries.size() - 1){ //Last iteration
                head->back = ringEntryNode;
                ringEntryNode->next = head;
            }else{ //Middle iteration
                ringEntryNode->back = prev;
                prev->next = ringEntryNode;
            }

            indexByNodeId.insert({ringEntryNode->entry.nodeId, ringEntryNode});

            prev = ringEntryNode;
        }

        return RingEntries{head, indexByNodeId};
    }
};