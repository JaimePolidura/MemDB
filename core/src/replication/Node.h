#pragma once

#include <string>
#include <nlohmann/json.hpp>

#include "NodeState.h"

struct Node {
public:
    std::string address;
    NodeState state;
    int nodeId;

    static bool canSendRequestUnicast(const Node& node) {
        return node.state == NodeState::RUNNING;
    }

    static std::string toJson(const Node& node) {
        return "{\"nodeId\": \""+std::to_string(node.nodeId)+"\", \"address\": \""+node.address+"\", \"state\": \""+NodeStates::parseNodeStateToString(node.state)+"\"}";
    }

    static Node fromJson(const nlohmann::json& json) {
        return Node{
                .address = json["address"].get<std::string>(),
                .state = NodeStates::parseNodeStateFromString(json["state"].get<std::string>()),
                .nodeId = json["nodeId"].get<int>(),
        };
    }
};