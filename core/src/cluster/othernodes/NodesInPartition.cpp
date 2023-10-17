#include "cluster/othernodes/NodesInPartition.h"

std::set<memdbNodeId_t> NodesInPartition::getAll() {
    return this->nodes;
}

void NodesInPartition::add(memdbNodeId_t nodeId) {
    this->nodes.insert(nodeId);
}

void NodesInPartition::remove(memdbNodeId_t nodeId) {
    this->nodes.erase(nodeId);
}

int NodesInPartition::size() {
    return this->nodes.size();
}