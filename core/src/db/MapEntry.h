#pragma once

#include "utils/strings/SimpleString.h"

template<typename SizeValue>
struct MapEntry {
    SimpleString<SizeValue> key;
    SimpleString<SizeValue> value;
    uint32_t keyHash;

    MapEntry(const SimpleString<SizeValue>& key, const SimpleString<SizeValue>& value, uint32_t keyHash):
            keyHash(keyHash),
            value(value),
            key(key) {}

    bool hasValue() {
        return this->value.data() != nullptr;
    }
};