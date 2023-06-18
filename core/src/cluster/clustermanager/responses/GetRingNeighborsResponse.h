#pragma once

#include "cluster/Node.h"

class GetRingNeighborsResponse {
public:
    std::vector<node_t> neighbors;

    static GetRingNeighborsResponse fromJson(const nlohmann::json& json) {
        std::vector<node_t> neighbors;
        auto jsonOtherNodes = json["neighbors"];
        for (const auto& otherNodeJson : jsonOtherNodes) {
            auto node = Node::fromJson(otherNodeJson);
            neighbors.push_back(node);
        }

        return GetRingNeighborsResponse{
                .neighbors = neighbors,
        };
    }
};