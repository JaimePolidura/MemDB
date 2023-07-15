#pragma once

#include "shared.h"

enum NodeState {
    RUNNING,
    SHUTDOWN,
    BOOTING
};

class NodeStates {
public:
    //False case: Can accept but cannot execute request yet
    static bool cantExecuteRequest(NodeState state);

    //False case: Can accept any request
    static bool canAcceptRequest(NodeState state);

    static std::string parseNodeStateToString(NodeState toParse);

    static NodeState parseNodeStateFromString(const std::string& toParse);
};