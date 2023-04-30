#pragma once

#include "cluster/partitions/RingEntry.h"
#include "cluster/Node.h"

struct RingEntryNode {
    RingEntry entry;

    RingEntryNode * next;
    RingEntryNode * back;

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

    uint32_t getDistance(memdbNodeId_t nodeA, memdbNodeId_t nodeB) {
        uint32_t size = this->indexByNodeId.size();

        auto pointerNodeA = this->indexByNodeId.at(nodeA);
        auto pointerNodeB = this->indexByNodeId.at(nodeB);
        uint32_t iterations = 0;

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
            RingEntryNode * ringEntryNode = new RingEntryNode();
            ringEntryNode->entry = entries[i];

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

        return RingEntries(head, indexByNodeId);
    }
};