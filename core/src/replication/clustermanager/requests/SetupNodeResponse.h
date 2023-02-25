#pragma once

#include <nlohmann/json.hpp>
#include <vector>

#include "replication/Node.h"

struct SetupNodeResponse {
public:
    std::vector<Node> nodes;
    int nodeId;

    static SetupNodeResponse fromJson(const nlohmann::json& json) {
        int nodeId = json["nodeId"].get<int>();

        std::vector<Node> allNodes;
        auto jsonNodes = json["nodes"];
        for (const auto& nodeJson : jsonNodes) {
            allNodes.push_back(Node{
                    .address = nodeJson["address"].get<std::string>(),
                    .state = parseNodeStateFromString(nodeJson["state"].get<std::string>()),
                    .nodeId = nodeJson["nodeId"].get<int>(),
            });
        }

        return SetupNodeResponse{
                .nodes = allNodes,
                .nodeId = nodeId,
        };
    }
};

