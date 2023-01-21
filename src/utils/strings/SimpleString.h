#pragma once

#include <cstdint>

/**
 * Apparently you cannot create an string from already heap allocated char *. So we create this class
 */
class SmallString {
public:
    char * value;
    uint8_t size;
    uint8_t * refCount;
public:
    SmallString(char * value, uint8_t size): value(value), size(size), refCount(new uint8_t(1)) {}

    SmallString(char * value, uint8_t size, uint8_t enable_shared_from_this ): value(value), size(size), refCount(new uint8_t(1)) {}

    SmallString() = default;

    void increaseRefCount() {
        this->refCount++;
    }

    void decreaseRefCount() {
        if(--this->refCount == 0)
            delete[] this->value;
    }

    char * operator[](int index) const {
        return this->value + index;
    }
};