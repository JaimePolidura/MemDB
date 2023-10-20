#pragma once

#include "shared.h"
#include "memdbtypes.h"

#include "utils/Utils.h"

#include "cluster/Node.h"

struct AllNodesResponse {
    std::vector<node_t> nodes;

    std::vector<node_t> getAllNodesExcept(memdbNodeId_t nodeIdExcept);

    node_t getNodeById(memdbNodeId_t nodeId);
};