#pragma once

#include <cstdint>
#include <string>
#include <optional>
#include <tgmath.h>
#include <atomic>

#include "utils/datastructures/tree/AVLTree.h"
#include "utils/threads/ReadWriteLock.h"

struct MapEntry {
    SimpleString key;
    uint32_t keyHash;
    SimpleString value;

    MapEntry(const SimpleString& key, uint32_t keyHash, const SimpleString& value):
        keyHash(keyHash),
        value(value),
        key(key) {}

    bool hasValue() {
        return this->value.data() != nullptr;
    }
};

class shared_mutex;

class Map {
private:
    std::atomic_uint32_t size;
    std::vector<AVLTree> buckets;
    std::vector<ReadWriteLock *> locks;
    uint16_t numberBuckets;

public:
    Map(uint16_t numberBuckets);

    /**
     * Returns true if operation was successful
     */
    bool put(const SimpleString& key, const SimpleString& value, bool ignoreTimeStamps, uint64_t timestamp, uint16_t nodeId);

    std::optional<MapEntry> get(const SimpleString& key) const;

    /**
     * Returns true if operation was successful
     */
    bool remove(const SimpleString& key, bool ignoreTimeStamps, uint64_t timestamp, uint16_t nodeId);

    bool contains(const SimpleString& key) const;

    std::vector<MapEntry> all();

    int getSize() const;

private:
    static const uint8_t HASH_PRIME_FACTOR = 31;

    uint32_t calculateHash(const SimpleString& key) const {
        uint32_t hashCode = 0;

        for(int i = 0; i < key.size; i++)
            hashCode += (* key[i]) * std::pow(HASH_PRIME_FACTOR, i);

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
        const_cast<ReadWriteLock *>(this->locks.at(hashCode % numberBuckets))->lockRead();
    }

    void unlockRead(uint32_t hashCode) const {
        const_cast<ReadWriteLock *>(this->locks.at(hashCode % numberBuckets))->unlockRead();
    }

    void lockWrite(uint32_t hashCode) const {
        const_cast<ReadWriteLock *>(this->locks.at(hashCode % numberBuckets))->lockWrite();
    }

    void unlockWrite(uint32_t hashCode) const {
        const_cast<ReadWriteLock *>(this->locks.at(hashCode % numberBuckets))->unlockWrite();
    }
};