#pragma once

#include <string>

enum NodeState {
    RUNNING,
    SHUTDOWN,
    BOOTING
};

NodeState parseNodeStateFromString(const std::string& toParse) {
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