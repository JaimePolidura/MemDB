#pragma once

#include "shared.h"
#include "memdbtypes.h"

struct NodeGroup {
private:
    std::set<memdbNodeId_t> nodes{};

public:
    NodeGroup() = default;

    std::set<memdbNodeId_t> getAll();

    void add(memdbNodeId_t nodeId);

    void remove(memdbNodeId_t nodeId);
    
    int size();
};