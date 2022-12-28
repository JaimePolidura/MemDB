#pragma once

#include <cstdint>
#include <string>
#include <optional>
#include <tgmath.h>

struct MapEntry {
    uint8_t * value;
    size_t valueSize;

    MapEntry(uint8_t * value, size_t valueSize): value(value), valueSize(valueSize) {}
};

class MapNode {
private:
    MapNode * next;
    uint8_t * value;
    size_t valueSize;
    int keyHash;
    bool deleted;

public:
    MapNode(uint8_t * newData, int newKeyHash, size_t newSize): keyHash{newKeyHash}, value{newData}, valueSize{newSize}, next{nullptr}, deleted{false} {}

    void replaceDeletedNode(int newHash, uint8_t * newData, size_t newSize) {
        this->value = newData;
        this->keyHash = newHash;
        this->valueSize = newSize;
        this->deleted = false;
    }

    void replaceValue(uint8_t * newData, size_t newSize) {
        this->value = newData;
        this->valueSize = newSize;
    }

    void setDeleted() {
        this->deleted = true;
        this->deleteDataPointer();
    }

    void setNext(MapNode * newNext) {
        this->next = newNext;
    }

    MapEntry toEntry() const { return MapEntry{value, valueSize}; }
    MapNode * getNext() const { return this->next; }
    bool isNotDeleted() const { return !this->deleted; }
    bool isDeleted() const { return this->deleted; }
    bool hasNotSameHash(int otherHashKey) const { return this->keyHash != otherHashKey; }
    bool hasSameHash(int otherKeyHash) const { return this->keyHash == otherKeyHash; }
    bool hasNoNext() const { return this->next == nullptr; }
    bool hasNext() const { return this->next != nullptr; }

private:
    void deleteDataPointer() {
        for(int i = 0; i <= this->valueSize / sizeof(this->value); i++)
            delete (this->value + i);
    }
};

class Map {
private:
    static const int NUMBER_OF_BUCKETS = 64;

    MapNode * buckets[NUMBER_OF_BUCKETS] = {};
    int size;

public:
    Map();

    void put(const std::string &key, uint8_t * value, size_t valueSize);

    std::optional<MapEntry> get(const std::string &key) const;

    void remove(const std::string &key);

    bool contains(const std::string &key) const;

    int getSize() const;

private:
    static const int HASH_PRIME_FACTOR = 31;

    int calculateHash(const std::string& key) const {
        int hashCode = 0;

        for(int i = 0; i < key.length(); i++)
            hashCode += key[i] * std::pow(HASH_PRIME_FACTOR, i);

        return hashCode;
    }

    MapNode * getNodeByKeyHash(int keyHash) const {
        MapNode * actualMapNode = this->getBucket(keyHash);

        while (actualMapNode != nullptr && (actualMapNode->hasNext() || actualMapNode->isNotDeleted()))
            if(actualMapNode->hasSameHash(keyHash) && actualMapNode->isNotDeleted())
                return actualMapNode;

        return (actualMapNode != nullptr && actualMapNode->hasSameHash(keyHash) && actualMapNode->isNotDeleted()) ?
               actualMapNode :
               nullptr;
    }

    MapNode * getBucket(int keyHash) const {
        return this->buckets[this->getBucketSlot(keyHash)];
    }

    int getBucketSlot(int keyHash) const{
        return keyHash % NUMBER_OF_BUCKETS;
    }
};