#pragma once

#include <cstdint>

/**
 * Apparently you cannot create an string from already heap allocated char *. So we create this class
 */
class SmallString {
public:
    char * value;
    uint8_t size;
    uint8_t timesToBeDeleted;
public:

    SmallString(char * value, uint8_t size): value(value), size(size), timesToBeDeleted(0) {}

    SmallString(char * value, uint8_t size, uint8_t timesToBeDeleted): value(value), size(size), timesToBeDeleted(timesToBeDeleted) {}

    SmallString() = default;

    void setDeleted() {
        if(--this->timesToBeDeleted == 0)
            delete[] this->value;
    }

    char * operator[](int index) const {
        return this->value + index;
    }
};