#pragma once

#include "shared.h"

#include "utils/datastructures/tree/AVLTree.h"
#include "utils/threads/SharedLock.h"
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
    std::vector<SharedLock *> locks;
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
        const_cast<SharedLock *>(this->locks.at(hashCode % numberBuckets))->lockShared();
    }

    void unlockRead(uint32_t hashCode) const {
        const_cast<SharedLock *>(this->locks.at(hashCode % numberBuckets))->unlockShared();
    }

    void lockWrite(uint32_t hashCode) const {
        const_cast<SharedLock *>(this->locks.at(hashCode % numberBuckets))->lockExclusive();
    }

    void unlockWrite(uint32_t hashCode) const {
        const_cast<SharedLock *>(this->locks.at(hashCode % numberBuckets))->unlockExclusive();
    }
};

using memDbDataStore_t = std::shared_ptr<Map<memDbDataLength_t>>;

template<typename SizeValue>
Map<SizeValue>::Map(uint16_t numberBuckets): numberBuckets(numberBuckets) {
    buckets.reserve(numberBuckets);

    for (int i = 0; i < numberBuckets; i++) {
        buckets.emplace_back();
        locks.push_back(new SharedLock());
    }
}

template<typename SizeValue>
bool Map<SizeValue>::put(const SimpleString<SizeValue>& key, const SimpleString<SizeValue>& value, bool ignoreTimestamps, uint64_t timestamp, uint16_t nodeId) {
    uint32_t keyHash = this->calculateHash(key);

    lockWrite(keyHash);

    AVLTree<SizeValue> * bucket = this->getBucket(keyHash);
    bool areadyContained = bucket->contains(keyHash);
    bool added = bucket->add(key, keyHash, value, ignoreTimestamps, timestamp, nodeId);

    if(added && !areadyContained) {
        this->size++;
    }

    unlockWrite(keyHash);

    return added;
}

template<typename SizeValue>
std::vector<MapEntry<SizeValue>> Map<SizeValue>::all() {
    std::vector<MapEntry<SizeValue>> all{};

    for (const AVLTree bucket: this->buckets){
        for (const auto node : bucket.all()){
            all.push_back(MapEntry{node->key, node->keyHash, node->value});
        }
    }

    return all;
}

template<typename SizeValue>
std::optional<MapEntry<SizeValue>> Map<SizeValue>::get(const SimpleString<SizeValue>& key) const {
    uint32_t hash = this->calculateHash(key);

    lockRead(hash);

    AVLNode<SizeValue> * node = this->getNodeByKeyHash(hash);
    
    const std::optional<MapEntry<SizeValue>> response = (node != nullptr ?
                                                         std::optional<MapEntry<SizeValue>>{MapEntry{node->key, node->keyHash, node->value}} :
                                                         std::nullopt);
    unlockRead(hash);

    return response;
}

template<typename SizeValue>
bool Map<SizeValue>::remove(const SimpleString<SizeValue>& key, bool ignoreTimestamps, uint64_t timestamp, uint16_t nodeId) {
    uint32_t hash = this->calculateHash(key);

    lockWrite(hash);

    AVLTree<SizeValue> * bucket = this->getBucket(hash);
    bool removed = false;
    if(bucket->contains(hash) && (removed = bucket->remove(hash, ignoreTimestamps, timestamp, nodeId))) {
        this->size--;
    }

    unlockWrite(hash);

    return removed;
}

template<typename SizeValue>
bool Map<SizeValue>::contains(const SimpleString<SizeValue>& key) const {
    return this->getNodeByKeyHash(this->calculateHash(key)) != nullptr;
}

template<typename SizeValue>
int Map<SizeValue>::getSize() const {
    return this->size;
}