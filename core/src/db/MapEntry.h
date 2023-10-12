#pragma once

#include "utils/strings/SimpleString.h"
#include "utils/clock/LamportClock.h"

template<typename SizeValue>
struct MapEntry {
    SimpleString<SizeValue> key;
    SimpleString<SizeValue> value;
    uint32_t keyHash{};
    LamportClock timestamp;

    MapEntry() = default;

    MapEntry(SimpleString<SizeValue> key, SimpleString<SizeValue> value, uint32_t keyHash, LamportClock timestamp):
        key(key), value(value), keyHash(keyHash), timestamp(timestamp) {}

    MapEntry(const MapEntry& other): key(other.key), value(other.value), keyHash(other.keyHash), timestamp(other.timestamp) {}

    bool hasValue() {
        return this->value.data() != nullptr;
    }
};

template<typename SizeValue>
using mapEntries_t = std::vector<MapEntry<SizeValue>>;