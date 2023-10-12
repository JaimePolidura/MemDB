#pragma once

#include "shared.h"

#include "AVLTree.h"
#include "utils/threads/SharedLock.h"
#include "utils/Iterator.h"
#include "memdbtypes.h"
#include "MapEntry.h"


template<typename SizeValue>
class Map {
private:
    std::atomic_uint32_t size;
    std::vector<AVLTree<SizeValue>> buckets;
    std::vector<SharedLock *> locks;
    uint16_t numberBuckets;
    friend class BucketMapHashOrderedIterator;

public:
    Map(uint16_t numberBuckets);

    /**
     * Returns true if operation was successful
     */
    bool put(const SimpleString<SizeValue>& key, const SimpleString<SizeValue>& value, bool ignoreTimeStamps, uint64_t timestamp, uint16_t nodeId);

    std::optional<MapEntry<SizeValue>> get(const SimpleString<SizeValue>& key) const;

    void clear();

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

public:
    class BucketMapHashOrderedIterator : public Iterator<std::vector<MapEntry<memDbDataLength_t>>> {
    private:
        Map<SizeValue> * map;
        uint32_t actualBucket;

    public:
        BucketMapHashOrderedIterator(Map * map): map(map), actualBucket(0) {}

        std::vector<MapEntry<memDbDataLength_t>> next() override {
            return this->map->buckets.at(this->actualBucket++).getOrderedByHash();
        }

        bool hasNext() override {
            return this->actualBucket >= this->map->numberBuckets;
        }

        uint64_t totalSize() override {
            return this->map->numberBuckets;
        }
    };

    BucketMapHashOrderedIterator bucketIterator() {
        return BucketMapHashOrderedIterator{this};
    }
};

using memDbDataStoreMap_t = std::shared_ptr<Map<memDbDataLength_t>>;

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
    bool alreadyContained = bucket->contains(keyHash);
    bool added = bucket->add(key, keyHash, value, ignoreTimestamps, timestamp, nodeId);

    if(added && !alreadyContained) {
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
            all.push_back(MapEntry{.key = node->key, .value = node->value, .keyHash = node->keyHash, .timestamp = node->timestamp});
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
            std::optional<MapEntry<SizeValue>>{MapEntry{.key = node->key, .value = node->value, .keyHash = node->keyHash, .timestamp = node->timestamp}} :
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
void Map<SizeValue>::clear() {
    for(int i = 0; i < numberBuckets; i++){
        this->locks[i]->lockExclusive();
        this->buckets[i].clear();
        this->locks[i]->unlockExclusive();
    }
}

template<typename SizeValue>
bool Map<SizeValue>::contains(const SimpleString<SizeValue>& key) const {
    return this->getNodeByKeyHash(this->calculateHash(key)) != nullptr;
}

template<typename SizeValue>
int Map<SizeValue>::getSize() const {
    return this->size;
}