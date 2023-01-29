#pragma once

#include <cstdint>
#include <atomic>
#include <mutex>

/**
 * Apparently you cannot create an string from already heap allocated char *. So we create this class
 */
class SimpleString {
public:
    uint8_t * value;
    volatile std::atomic_int32_t * refCount;
    uint8_t size;
public:
    SimpleString(uint8_t * value, std::atomic_int32_t * refCount, uint8_t size): value(value), refCount(refCount), size(size) {}

    SimpleString(uint8_t * value, uint8_t size): value(value), size(size), refCount(new std::atomic_int32_t(1)) {}

    void increaseRefCount() {
        if(!this->refCount && this->refCount->load() < 0)
            return;

        this->refCount->fetch_add(1);
    }

    void decreaseRefCount() {
        if(this->isDeleted() || this->refCount->load() < 1)
            return;

        bool success = this->atomicDecrementRefcount();

        if(success){
            delete[] this->value;
            delete this->refCount;
            this->value = nullptr;
            this->refCount = nullptr;
        }
    }

    uint8_t * operator[](int index) const {
        return this->value + index;
    }

    bool isDeleted() {
        return this->value == nullptr || this->refCount == nullptr;
    }

    static SimpleString empty() {
        return SimpleString{nullptr, nullptr, 0};
    }

    static SimpleString fromString(std::string&& string) {
        uint8_t * valuePtr = new uint8_t[string.size()];
        for (int i = 0; i < string.size(); ++i)
            * (valuePtr + i) = * (string.begin() + i);

        return SimpleString{valuePtr, static_cast<uint8_t>(string.size())};
    }

    static SimpleString fromArray(std::initializer_list<uint8_t> values) {
        uint8_t * valuePtr = new uint8_t[values.size()];
        for (int i = 0; i < values.size(); ++i)
            * (valuePtr + i) = * (values.begin() + i);

        return SimpleString{valuePtr, static_cast<uint8_t>(values.size())};
    }

    static SimpleString fromChar(char&& value) {
        char * valuePtr = new char();
        * valuePtr = value;

        return SimpleString{(uint8_t *) valuePtr, 1};
    }

private:
    bool atomicDecrementRefcount() {
        int32_t expected;
        bool success = false;

        do {
            expected = this->refCount->load();
        } while (expected > 0 && !(success = this->refCount->compare_exchange_strong(expected, expected - 1)));

        return expected - 1 == 0 && success;
    }
};