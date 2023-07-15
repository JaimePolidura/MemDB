#include "cluster/clustermanager/responses/AllNodesResponse.h"

std::vector<node_t> AllNodesResponse::getAllNodesExcept(memdbNodeId_t nodeIdExcept) {
    return Utils::filter<node_t>(this->nodes, [nodeIdExcept](const node_t& it) -> bool {
        return it->nodeId != nodeIdExcept;
    });
}

node_t AllNodesResponse::getNodeById(memdbNodeId_t nodeId) {
    for (const node_t& node: this->nodes) {
        if(node->nodeId == nodeId){
            return node;
        }
    }

    throw std::runtime_error("Node not found in AllNodesResponse");
}

AllNodesResponse AllNodesResponse::fromJson(const nlohmann::json& json) {
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