#pragma once

#include "cluster/Node.h"

class GetRingNeighborsResponse {
public:
    std::vector<node_t> neighbors;
    
    static GetRingNeighborsResponse fromJson(const nlohmann::json& json);
};