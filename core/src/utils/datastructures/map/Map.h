#pragma once

#include <cstdint>
#include <string>
#include <optional>
#include <tgmath.h>
#include <atomic>

#include "utils/datastructures/tree/AVLTree.h"
#include "utils/threads/ReadWriteLock.h"
#include "memdbtypes.h"

template<typename SizeValue>
struct MapEntry {
    SimpleString<SizeValue> key;
    uint32_t keyHash;
    SimpleString<SizeValue> value;

    MapEntry(const SimpleString<SizeValue>& key, uint32_t keyHash, const SimpleString<SizeValue>& value):
        keyHash(keyHash),
        value(value),
        key(key) {}

    bool hasValue() {
        return this->value.data() != nullptr;
    }
};

template<typename SizeValue>
class Map {
private:
    std::atomic_uint32_t size;
    std::vector<AVLTree<SizeValue>> buckets;
    std::vector<ReadWriteLock *> locks;
    uint16_t numberBuckets;

public:
    Map(uint16_t numberBuckets);

    /**
     * Returns true if operation was successful
     */
    bool put(const SimpleString<SizeValue>& key, const SimpleString<SizeValue>& value, bool ignoreTimeStamps, uint64_t timestamp, uint16_t nodeId);

    std::optional<MapEntry<SizeValue>> get(const SimpleString<SizeValue>& key) const;

    /**
     * Returns true if operation was successful
     */
    bool remove(const SimpleString<SizeValue>& key, bool ignoreTimeStamps, uint64_t timestamp, uint16_t nodeId);

    bool contains(const SimpleString<SizeValue>& key) const;

    std::vector<MapEntry<SizeValue>> all();

    int getSize() const;

private:
    static const uint8_t HASH_PRIME_FACTOR = 31;

    uint32_t calculateHash(const SimpleString<SizeValue>& key) const {
        uint32_t hashCode = 0;

        for(int i = 0; i < key.size; i++)
            hashCode += (* key[i]) * std::pow(HASH_PRIME_FACTOR, i);

        return hashCode;
    }

    AVLNode<SizeValue> * getNodeByKeyHash(uint32_t keyHash) const {
        AVLTree<SizeValue> * actualMapNode = this->getBucket(keyHash);

        return actualMapNode->get(keyHash);
    }

    AVLTree<SizeValue> * getBucket(uint32_t keyHash) const {
        return const_cast<AVLTree<SizeValue> *>(this->buckets.data()) + (keyHash % numberBuckets);
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

using memDbDataStore_t = std::shared_ptr<Map<defaultMemDbSize_t>>;
