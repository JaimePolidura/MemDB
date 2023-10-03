#include "RingQueue.h"

void RingQueue::enqueue(OperationBody &operation) {
    RingQueueNode ** slot = this->slots[std::hash<std::thread::id>{}(std::this_thread::get_id())];
    RingQueueNode * nodeInSlot = *slot;
    RingQueueNode * newNode = new RingQueueNode(operation, false);

    if(nodeInSlot == nullptr){
        RingQueueNode * sentinel = reinterpret_cast<RingQueueNode *>(
                reinterpret_cast<std::uintptr_t>(*slot) - offsetof(RingQueueNode, prev)
        );

        sentinel->prev = newNode;

        return;
    }

    while(nodeInSlot->prev != nullptr){
        nodeInSlot = nodeInSlot->prev;
    }

    nodeInSlot->prev = newNode;
}

OperationBody RingQueue::dequeue(int slotIndex) {
    RingQueueNode ** slot = this->slots[std::hash<std::thread::id>{}(std::this_thread::get_id())];
    RingQueueNode * nodeInSlot = *slot;

    if(nodeInSlot != nullptr &&
        nodeInSlot->operationBody.getTimestamp() + 1 == this->lastSeen){

        OperationBody operationBody = nodeInSlot->operationBody;
        this->lastSeen++;
        this->slots[slotIndex] = &nodeInSlot->prev;

        return operationBody;
    }
}

RingQueue::RingQueue() {
    //Create sentinel nodes
}

RingQueueNode::RingQueueNode(const OperationBody& operationBody, bool sentinel): operationBody(operationBody), sentinel(sentinel) {}
