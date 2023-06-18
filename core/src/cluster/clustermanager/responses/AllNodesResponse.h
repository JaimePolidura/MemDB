#pragma once

#include "shared.h"
#include "memdbtypes.h"

#include "utils/Utils.h"

#include "cluster/Node.h"

struct AllNodesResponse {
    std::vector<node_t> nodes;

    std::vector<node_t> getAllNodesExcept(memdbNodeId_t nodeIdExcept) {
        return Utils::filter<node_t>(this->nodes, [nodeIdExcept](const node_t& it) -> bool {
            return it->nodeId != nodeIdExcept;
        });
    }

    node_t getNodeById(memdbNodeId_t nodeId) {
        for (const node_t& node: this->nodes) {
            if(node->nodeId == nodeId){
                return node;
            }
        }

        throw std::runtime_error("Node not found in AllNodesResponse");
    }

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