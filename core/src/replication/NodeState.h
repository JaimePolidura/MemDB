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
    static bool cantExecuteRequest(NodeState state) {
        return state == NodeState::RUNNING;
    }

    //False case: Can accept any request
    static bool canAcceptRequest(NodeState state) {
        return state != NodeState::SHUTDOWN;
    }

    static std::string parseNodeStateToString(NodeState toParse) {
        if(toParse == NodeState::RUNNING) {
            return "RUNNING";
        }else if(toParse == NodeState::SHUTDOWN) {
            return "SHUTDOWN";
        }else if (toParse == NodeState::BOOTING) {
            return "BOOTING";
        }else {
            throw std::runtime_error("Unknown node state for " + toParse);
        }
    }

    static NodeState parseNodeStateFromString(const std::string& toParse) {
        if(toParse == "RUNNING") {
            return NodeState::RUNNING;
        }else if(toParse == "SHUTDOWN") {
            return NodeState::SHUTDOWN;
        }else if (toParse == "BOOTING") {
            return NodeState::BOOTING;
        }else {
            throw std::runtime_error("Unknown node state for " + toParse);
        }
    }
};