#pragma once

#include "utils/strings/SimpleString.h"

template<typename SizeValue>
struct MapEntry {
    SimpleString<SizeValue> key;
    SimpleString<SizeValue> value;
    uint32_t keyHash{};
    LamportClock timestamp;

    MapEntry() = default;

    MapEntry(const MapEntry& other): key(other.key), value(other.value), keyHash(other.keyHash), timestamp(other.timestamp) {}

    bool hasValue() {
        return this->value.data() != nullptr;
    }
};

template<typename SizeValue>
using mapEntries_t = std::vector<MapEntry<SizeValue>>;