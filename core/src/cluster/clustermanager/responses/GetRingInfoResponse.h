#pragma once

#include "cluster/partitions/RingEntry.h"
#include "shared.h"

struct GetRingInfoResponse {
    std::vector<RingEntry> entries;
    uint32_t nodesPerPartition;
    uint32_t maxSize;

    static GetRingInfoResponse fromJson(const nlohmann::json& json);
};