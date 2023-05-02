#pragma once

#include "shared.h"

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

    bool hasData() const {
        return this->size != 0 || this->value.get() != nullptr;
    }

    std::string toString() const {
        return std::string((char *) this->value.get(), this->size);
    }

    //TODO Avoid copy. See std::span
    std::vector<uint8_t> toVector() const {
        return std::vector<uint8_t>(this->value.get(), this->value.get() + this->size);
    }

    uint8_t * operator[](int index) const {
        return this->value.get() + index;
    }

    uint8_t * operator+(int other) const {
        return this->value.get() + other;
    }

    bool operator<(const SimpleString<StringLengthType>& a) const {
        return std::strcmp((char *) this->value.get(), (char *) a.value.get()) < 0;
    }

    size_t getSizeOfStringLengthType() const {
        return sizeof(StringLengthType);
    }

    static SimpleString empty() {
        return SimpleString{nullptr, 0};
    }

    template<typename T>
    static SimpleString<StringLengthType> fromNumber(const T& from) {
        static_assert(std::is_arithmetic<T>::value, "T must be a number type");

        uint8_t * valuePtr = new uint8_t[sizeof(T)];
        for(int i = 0; i < sizeof(T); i++){
            * valuePtr = from >> (i * 8);
        }

        return SimpleString<StringLengthType>{valuePtr, static_cast<StringLengthType>(sizeof(T))};
    }

    static SimpleString<StringLengthType> fromString(std::string&& string) {
        uint8_t * valuePtr = new uint8_t[string.size()];
        for (int i = 0; i < string.size(); ++i)
            * (valuePtr + i) = * (string.begin() + i);

        return SimpleString<StringLengthType>{valuePtr, static_cast<StringLengthType>(string.size())};
    }

    static SimpleString<StringLengthType> fromArray(std::initializer_list<uint8_t> values) {
        return fromVector(values);
    }

    static SimpleString<StringLengthType> fromVector(const std::vector<uint8_t>& values) {
        uint8_t * valuePtr = new uint8_t[values.size()];
        for (int i = 0; i < values.size(); ++i)
            * (valuePtr + i) = * (values.begin() + i);

        return SimpleString<StringLengthType>{valuePtr, static_cast<StringLengthType>(values.size())};
    }

    static SimpleString fromChar(const char& value) {
        char * valuePtr = new char();
        * valuePtr = value;

        return SimpleString{(uint8_t *) valuePtr, 1};
    }
};

template <typename StringLengthType>
struct SimpleStringHash {
    std::size_t operator()(const SimpleString<StringLengthType>& str) const {
        // Compute a hash value for the string
        std::size_t seed = 0;
        for (StringLengthType i = 0; i < str.size; ++i) {
            seed ^= *(str.data() + i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

template <typename StringLengthType>
struct SimpleStringEqual {
    bool operator()(const SimpleString<StringLengthType>& lhs, const SimpleString<StringLengthType>& rhs) const {
        // Compare two strings for equality
        if (lhs.size != rhs.size) {
            return false;
        }
        for (StringLengthType i = 0; i < lhs.size; ++i) {
            if (* (lhs.data() + i) != * (rhs.data() + i)) {
                return false;
            }
        }
        return true;
    }
};