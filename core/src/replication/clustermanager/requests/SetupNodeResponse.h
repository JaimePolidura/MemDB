#pragma once

#include <nlohmann/json.hpp>
#include <vector>

#include "replication/Node.h"

struct SetupNodeResponse {
public:
    std::vector<Node> otherNodes;
    Node self;

    static SetupNodeResponse fromJson(const nlohmann::json& json) {
        auto self = Node::fromJson(json["self"]);

        std::vector<Node> otherNodes;
        auto jsonOtherNodes = json["nodes"];
        for (const auto& otherNodeJson : jsonOtherNodes)
            otherNodes.push_back(Node::fromJson(otherNodeJson));

        return SetupNodeResponse{
                .otherNodes = otherNodes,
                .self = self,
        };
    }
};

