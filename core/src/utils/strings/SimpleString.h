#pragma once

#include <cstdint>
#include <atomic>
#include <mutex>
#include <memory>

/**
 * Apparently you cannot create an string from already heap allocated char *. So we create this class
 */
class SimpleString {
private:
    std::shared_ptr<uint8_t> value;
public:
    uint8_t size;

    SimpleString(uint8_t * value, uint8_t size): value(value), size(size) {}

    uint8_t * data() const {
        return this->value.get();
    }

    uint8_t * operator[](int index) const {
        return this->value.get() + index;
    }

    uint8_t * operator+(int other) const {
        return this->value.get() + other;
    }

    static SimpleString empty() {
        return SimpleString{nullptr, 0};
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
};