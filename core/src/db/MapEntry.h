#pragma once

#include "utils/strings/SimpleString.h"
#include "utils/clock/LamportClock.h"
#include "db/NodeType.h"

template<typename SizeValue>
struct MapEntry {
    SimpleString<SizeValue> key;
    uint32_t keyHash{};
    std::shared_ptr<void> data;
    NodeType type;

    MapEntry() = default;

    MapEntry(SimpleString<SizeValue> key, uint32_t keyHash, NodeType type, std::shared_ptr<void> data):
        key(key), keyHash(keyHash), data(data), type(type) {}

    std::shared_ptr<CounterAVLNode> toCounter() const {
        return std::dynamic_pointer_cast<CounterAVLNode>(this->data);
    }

    std::shared_ptr<DataAVLNode<SizeValue>> toData() const {
        return std::dynamic_pointer_cast<DataAVLNode<SizeValue>>(this->data);
    }
};

template<typename SizeValue>
using mapEntries_t = std::vector<MapEntry<SizeValue>>;