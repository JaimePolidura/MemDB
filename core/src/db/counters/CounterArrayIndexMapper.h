#pragma once

#include "memdbtypes.h"
#include "shared.h"

class CounterArrayIndexMapper {
public:
    int getIndex(memdbNodeId_t nodeId);

private:
    std::map<memdbNodeId_t, int> indexByNodeId{};
    int lastIndexAssigned{-1};
};
