#include "cluster/NodeState.h"

bool NodeStates::cantExecuteRequest(NodeState state) {
    return state == NodeState::RUNNING;
}

//False case: Can accept any request
bool NodeStates::canAcceptRequest(NodeState state) {
    return state != NodeState::SHUTDOWN;
}

std::string NodeStates::parseNodeStateToString(NodeState toParse) {
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

NodeState NodeStates::parseNodeStateFromString(const std::string& toParse) {
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