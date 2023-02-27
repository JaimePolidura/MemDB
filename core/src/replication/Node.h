#pragma once

#include <string>

#include "NodeState.h"

struct Node {
    std::string address;
    NodeState state;
    int nodeId;
};

class Nodes {
public:
    static bool canSendRequestUnicast(const Node& node) {
        return node.state == NodeState::RUNNING;
    }
};
