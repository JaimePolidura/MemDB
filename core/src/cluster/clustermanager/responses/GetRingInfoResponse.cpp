#include "cluster/clustermanager/responses/GetRingInfoResponse.h"

GetRingInfoResponse GetRingInfoResponse::fromJson(const nlohmann::json& json) {
    std::cout << "GetRingInfoResponse " << json.dump() << std::endl;

    std::vector<RingEntry> entries;

    uint32_t nodesPerPartition = json["nodesPerPartition"].get<uint32_t>();
    uint32_t maxSize = json["maxSize"].get<uint32_t>();

    auto jsonEntries = json["entries"];
    for (const auto& entryRingJson : jsonEntries) {
        auto entry = RingEntry::fromJson(entryRingJson);
        entries.push_back(entry);
    }

    return GetRingInfoResponse {
            .entries = entries,
            .nodesPerPartition = nodesPerPartition,
            .maxSize = maxSize
    };
}