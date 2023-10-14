#include "GetRingInfoResponse.h"

GetRingInfoResponse GetRingInfoResponse::fromJson(const nlohmann::json& json) {
    std::vector<RingEntry> entries;

    uint32_t nodesPerPartition = json["nodesPerPartition"].get<uint32_t>();
    uint32_t maxSize = json["maxSize"].get<uint32_t>();

    return GetRingInfoResponse {
            .nodesPerPartition = nodesPerPartition,
            .maxSize = maxSize
    };
}