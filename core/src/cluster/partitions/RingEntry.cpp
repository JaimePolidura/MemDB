#include "cluster/partitions/RingEntry.h"

RingEntry RingEntry::fromJson(const nlohmann::json& json) {
    RingEntry ringEntry{};
    ringEntry.nodeId = std::stoi(json["nodeId"].getAll<std::string>());
    ringEntry.ringPosition = json["ringPosition"].getAll<uint32_t>();

    return ringEntry;
}