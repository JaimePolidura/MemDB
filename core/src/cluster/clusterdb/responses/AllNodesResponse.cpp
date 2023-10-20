#include "AllNodesResponse.h"

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