#pragma once

#include "replication/partitions/RingEntry.h"
#include "shared.h"

struct RingEntriesResponse {
    std::vector<RingEntry> entries;

    static RingEntriesResponse fromJson(const nlohmann::json& json) {
        std::vector<RingEntry> entries;
        auto jsonEntries = json["entries"];
        for (const auto& entryRingJson : jsonEntries) {
            auto entry = RingEntry::fromJson(entryRingJson);
            entries.push_back(entry);
        }

        return RingEntriesResponse{
                .entries = entries,
        };
    }
};