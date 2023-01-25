#pragma once

#include <memory>

#include "utils/clock/LamportClock.h"

class ReplicationNode {
private:
    uint16_t nodeId;
    std::shared_ptr<LamportClock> clock;

public:
    ReplicationNode(uint16_t nodeId, std::shared_ptr<LamportClock> clock) : nodeId(nodeId), clock(clock) {}

    uint64_t tick(uint64_t other) {
        return this->clock->tick(other);
    }
};