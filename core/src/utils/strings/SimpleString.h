#pragma once

#include <cstdint>
#include <atomic>
#include <mutex>
#include <memory>

/**
 * Apparently you cannot create an string from already heap allocated char *. So we create this class
 */
template<typename StringLengthType>
class SimpleString {
    static_assert(std::is_unsigned_v<StringLengthType>, "StringLengthype should be unsigned");

private:
    std::shared_ptr<uint8_t> value;
public:
    StringLengthType size;

    SimpleString(uint8_t * value, StringLengthType size): value(value), size(size) {}

    uint8_t * data() const {
        return this->value.get();
    }

    uint8_t * operator[](int index) const {
        return this->value.get() + index;
    }

    uint8_t * operator+(int other) const {
        return this->value.get() + other;
    }
    
    size_t getSizeOfStringLengthType() const {
        return sizeof(StringLengthType);
    }

    static SimpleString empty() {
        return SimpleString{nullptr, 0};
    }

    static SimpleString<StringLengthType> fromString(std::string&& string) {
        uint8_t * valuePtr = new uint8_t[string.size()];
        for (int i = 0; i < string.size(); ++i)
            * (valuePtr + i) = * (string.begin() + i);

        return SimpleString<StringLengthType>{valuePtr, static_cast<StringLengthType>(string.size())};
    }

    static SimpleString<StringLengthType> fromArray(std::initializer_list<uint8_t> values) {
        uint8_t * valuePtr = new uint8_t[values.size()];
        for (int i = 0; i < values.size(); ++i)
            * (valuePtr + i) = * (values.begin() + i);

        return SimpleString<StringLengthType>{valuePtr, static_cast<StringLengthType>(values.size())};
    }

    static SimpleString fromChar(char&& value) {
        char * valuePtr = new char();
        * valuePtr = value;

        return SimpleString{(uint8_t *) valuePtr, 1};
    }
};