#pragma once

#include <string>

#include "NodeState.h"

struct Node {
    std::string address;
    NodeState state;
    int nodeId;
};
