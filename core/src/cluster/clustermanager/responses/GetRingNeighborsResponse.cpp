#include "cluster/clustermanager/responses/GetRingNeighborsResponse.h"

GetRingNeighborsResponse GetRingNeighborsResponse::fromJson(const nlohmann::json& json) {
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