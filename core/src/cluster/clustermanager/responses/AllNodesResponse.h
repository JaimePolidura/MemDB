#pragma once

#include "shared.h"

#include "cluster/Node.h"

struct AllNodesResponse {
    std::vector<node_t> nodes;

    static AllNodesResponse fromJson(const nlohmann::json& json) {
        std::vector<node_t> nodes;
        auto jsonOtherNodes = json["nodes"];
        for (const auto& otherNodeJson : jsonOtherNodes) {
            auto node = Node::fromJson(otherNodeJson);
            nodes.push_back(node);
        }

        return AllNodesResponse{
                .nodes = nodes,
        };
    }
};