#pragma once

#include <cstdint>
#include <atomic>

/**
 * Apparently you cannot create an string from already heap allocated char *. So we create this class
 */
class SimpleString {
public:
    uint8_t * value;
    std::atomic_uint8_t * refCount;
    uint8_t size;
public:
    SimpleString(): value(nullptr), refCount(0), size(0) {}

    SimpleString(uint8_t * value, uint8_t size): value(value), size(size), refCount(new std::atomic_uint8_t(0)) {}

    SimpleString(uint8_t * value, uint8_t size, uint8_t refCountCons): value(value), size(size), refCount(new std::atomic_uint8_t(refCountCons)) {}

    void increaseRefCount() {
        this->refCount++;
    }

    void decreaseRefCount() {
        if(--this->refCount == 0 && this->value)
            delete[] this->value;
    }

    uint8_t * operator[](int index) const {
        return this->value + index;
    }

    static SimpleString empty() {
        return SimpleString{};
    }
};