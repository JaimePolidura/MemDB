#pragma once

#include <memory>

#include "utils/clock/LamportClock.h"

class ReplicationNode {
private:
    uint16_t nodeId;
    LamportClock clock;

public:
    ReplicationNode(uint16_t nodeId) : nodeId(nodeId), clock(nodeId) {}

    uint64_t tick(uint64_t other) {
        return this->clock.tick(other);
    }
};