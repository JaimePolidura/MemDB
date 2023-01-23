#pragma once

#include <cstdint>
#include <atomic>

/**
 * Apparently you cannot create an string from already heap allocated char *. So we create this class
 */
class SimpleString {
public:
    uint8_t * value;
    std::atomic_int8_t * refCount;
    uint8_t size;
public:
    SimpleString(uint8_t * value, std::atomic_int8_t * refCount, uint8_t size): value(value), refCount(refCount), size(size) {}

    SimpleString(uint8_t * value, uint8_t size): value(value), size(size), refCount(new std::atomic_int8_t(1)) {}

    void increaseRefCount() {
        if(!this->refCount && this->refCount->load() < 0)
            return;

        this->refCount->fetch_add(1);
    }

    void decreaseRefCount() {
        if(this->refCount == nullptr || this->refCount->load() < 1)
            return;

        int attempts = this->atomicDecrementRefcount();

        if(attempts == 0){
            delete[] this->value;
            delete this->refCount;
        }
    }

    uint8_t * operator[](int index) const {
        return this->value + index;
    }

    static SimpleString empty() {
        return SimpleString{nullptr, nullptr, 0};
    }

private:
    int atomicDecrementRefcount() {
        int attempts = 0;
        int8_t expected;

        do {
            expected = this->refCount->load();
            attempts++;
        } while (!this->refCount->compare_exchange_weak(expected, expected - 1));

        return attempts;
    }
};