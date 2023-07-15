#pragma once

#include "shared.h"
#include "memdbtypes.h"

struct RingEntry {
public:
    memdbNodeId_t nodeId;
    uint32_t ringPosition;

    static RingEntry fromJson(const nlohmann::json& json);
};