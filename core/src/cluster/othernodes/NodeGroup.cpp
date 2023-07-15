#include "cluster/othernodes/NodeGroup.h"

std::set<memdbNodeId_t> NodeGroup::getAll() {
    return this->nodes;
}

void NodeGroup::add(memdbNodeId_t nodeId) {
    this->nodes.insert(nodeId);
}

void NodeGroup::remove(memdbNodeId_t nodeId) {
    this->nodes.erase(nodeId);
}

int NodeGroup::size() {
    return this->nodes.size();
}