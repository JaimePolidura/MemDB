#pragma once

#include "shared.h"
#include "memdbtypes.h"

struct RingEntry {
public:
    memdbNodeId_t nodeId;
    uint32_t ringPosition;

    static RingEntry fromJson(const nlohmann::json& json) {
        RingEntry ringEntry;
        ringEntry.nodeId = std::stoi(json["nodeId"].get<std::string>());
        ringEntry.ringPosition = json["ringPosition"].get<uint32_t>();

        return ringEntry;
    }
};