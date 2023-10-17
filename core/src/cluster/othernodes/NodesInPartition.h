#pragma once

#include "shared.h"
#include "memdbtypes.h"

struct NodesInPartition {
private:
    std::set<memdbNodeId_t> nodes{};

public:
    NodesInPartition() = default;

    std::set<memdbNodeId_t> getAll();

    void add(memdbNodeId_t nodeId);

    void remove(memdbNodeId_t nodeId);
    
    int size();
};