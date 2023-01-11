#pragma once

#include <cstdint>
#include <string>
#include <optional>
#include <tgmath.h>
#include <atomic>
#include <boost/thread/shared_mutex.hpp>

#include "../tree/AVLTree.h"

struct MapEntry {
    uint8_t * value;
    size_t valueSize;

    MapEntry(uint8_t * value, size_t valueSize): value(new uint8_t[valueSize]), valueSize(valueSize) {
        std::copy(value, value + valueSize, this->value);
    }
};

class Map {
private:
    std::atomic_uint32_t size;
    std::vector<AVLTree> buckets;
    std::vector<boost::shared_mutex *> locks;
    uint16_t numberBuckets;

public:
    Map(uint16_t numberBuckets);

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
        return const_cast<AVLTree *>(this->buckets.data()) + (keyHash % numberBuckets);
    }

    void lockRead(uint32_t hashCode) const {
        const_cast<boost::shared_mutex *>(this->locks.at(hashCode % numberBuckets))->lock_shared();
    }

    void unlockRead(uint32_t hashCode) const {
        const_cast<boost::shared_mutex *>(this->locks.at(hashCode % numberBuckets))->unlock_shared();
    }

    void lockWrite(uint32_t hashCode) const {
        const_cast<boost::shared_mutex *>(this->locks.at(hashCode % numberBuckets))->lock();

        printf("LOCKED %i -> %i, ", hashCode, hashCode % numberBuckets);
    }

    void unlockWrite(uint32_t hashCode) const {
        const_cast<boost::shared_mutex *>(this->locks.at(hashCode % numberBuckets))->unlock();

        printf(" UNLOCKED %i -> %i\n", hashCode, hashCode % numberBuckets);
    }
};