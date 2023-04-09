#pragma once

#include "shared.h"

#include "replication/Node.h"

struct AllNodesResponse {
    std::vector<Node> nodes;

    static AllNodesResponse fromJson(const nlohmann::json& json) {
        std::vector<Node> nodes;
        auto jsonOtherNodes = json["nodes"];
        for (const auto& otherNodeJson : jsonOtherNodes)
            nodes.push_back(Node::fromJson(otherNodeJson));

        return AllNodesResponse{
                .nodes = nodes,
        };
    }
};