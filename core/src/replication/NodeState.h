#pragma once

#include <string>

enum NodeState {
    RUNNING,
    SHUTDOWN,
    BOOTING
};

std::string parseNodeStateToString(NodeState toParse) {
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