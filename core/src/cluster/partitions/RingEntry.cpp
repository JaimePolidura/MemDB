#include "cluster/partitions/RingEntry.h"

RingEntry RingEntry::fromJson(const nlohmann::json& json) {
    RingEntry ringEntry{};
    ringEntry.nodeId = std::stoi(json["selfNode"].get<std::string>());
    ringEntry.ringPosition = json["ringPosition"].get<uint32_t>();

    return ringEntry;
}