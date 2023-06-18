#pragma once

#include "shared.h"
#include "memdbtypes.h"

struct NodeGroup {
private:
    std::set<memdbNodeId_t> nodes;

public:
    std::set<memdbNodeId_t> getAll() {
        return this->nodes;
    }

    void add(memdbNodeId_t nodeId) {
        this->nodes.insert(nodeId);
    }

    void remove(memdbNodeId_t nodeId) {
        this->nodes.erase(nodeId);
    }

    int size() {
        return this->nodes.size();
    }
};