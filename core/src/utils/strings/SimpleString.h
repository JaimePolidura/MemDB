#pragma once

#include "shared.h"
#include "utils/Utils.h"
#include "memdbtypes.h"

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

    SimpleString() = default;

    uint8_t * data() const {
        return this->value.get();
    }

    bool hasData() const {
        return this->size != 0 || this->value.get() != nullptr;
    }

    std::string toString() const {
        return std::string((char *) this->value.get(), this->size);
    }

    template<typename T>
    T to() const {
        return Utils::parse<T>(this->value.get());
    }

    //TODO Avoid copy. See std::span
    std::vector<uint8_t> toVector() const {
        return this->size > 0 ?
         std::vector<uint8_t>(this->value.get(), this->value.get() + this->size) :
         std::vector<uint8_t>{};
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

        auto * valuePtr = new uint8_t[sizeof(T)];
        for(int i = 0; i < sizeof(T); i++){
            size_t toMove = (sizeof(T) - 1 - i) * 8;
            * (valuePtr + i) = from >> toMove;
        }

        return SimpleString<StringLengthType>{valuePtr, static_cast<StringLengthType>(sizeof(T))};
    }

    template<typename T>
    static SimpleString<StringLengthType> fromPointer(T * ptr, std::size_t size) {
        uint8_t * ptrCastedU8 = static_cast<uint8_t *>(ptr);
        uint8_t * result = new uint8_t[size];

        std::copy(result, ptrCastedU8, ptrCastedU8 + size);
    
        return SimpleString<StringLengthType>{result, static_cast<StringLengthType>(size)};
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

    static SimpleString<StringLengthType> fromSimpleStrings(const std::vector<SimpleString<StringLengthType>>& values) {
        memDbDataLength_t totalSize = 0;
        for (const SimpleString<StringLengthType>& value: values) {
            totalSize += value.size;
        }

        uint8_t * result = new uint8_t[totalSize];

        int lastCopiedOffset = 0;

        for(const SimpleString<StringLengthType>& value : values){
            for(int i = 0; i < value.size; i++){
                *(result + lastCopiedOffset + i) = *(value.data() + i);
            }
            lastCopiedOffset += value.size;
        }        

        return SimpleString<StringLengthType>{result, totalSize};
    }

    static SimpleString<StringLengthType> fromVector(const std::vector<uint8_t>& values) {
        if(values.empty()){
            return SimpleString<StringLengthType>{new uint8_t(0x00), 0};
        }

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

using setSimpleString_t = std::unordered_set<SimpleString<memDbDataLength_t>, SimpleStringHash<memDbDataLength_t>, SimpleStringEqual<memDbDataLength_t>>;