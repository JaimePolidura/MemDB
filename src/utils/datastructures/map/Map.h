#pragma once

#include <cstdint>
#include <string>
#include <optional>
#include <tgmath.h>

#include "../tree/AVLTree.h"

struct MapEntry {
    uint8_t * value;
    size_t valueSize;

    MapEntry(uint8_t * value, size_t valueSize): value(value), valueSize(valueSize) {}
};


class Map {
private:
    static const int NUMBER_OF_BUCKETS = 64;

    AVLTree * buckets[NUMBER_OF_BUCKETS];
    uint32_t size;

public:
    Map(): size(0) {
        for (int i = 0; i < NUMBER_OF_BUCKETS; i++)
            buckets[i] = new AVLTree();
    };

    void put(const std::string &key, uint8_t * value, size_t valueSize);

    std::optional<MapEntry> get(const std::string &key) const;

    void remove(const std::string &key);

    bool contains(const std::string &key) const;

    int getSize() const;

private:
    static const uint8_t HASH_PRIME_FACTOR = 31;

    uint32_t calculateHash(const std::string& key) const {
        uint32_t hashCode = 0;

        for(int i = 0; i < key.length(); i++)
            hashCode += key[i] * std::pow(HASH_PRIME_FACTOR, i);

        return hashCode;
    }

    AVLNode * getNodeByKeyHash(uint32_t keyHash) const {
        AVLTree * actualMapNode = this->getBucket(keyHash);

        return actualMapNode->get(keyHash);
    }

    AVLTree * getBucket(uint32_t keyHash) const {
        return this->buckets[keyHash % NUMBER_OF_BUCKETS];
    }
};