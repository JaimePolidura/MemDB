#pragma once

#include "shared.h"

#include "AVLTree.h"
#include "utils/threads/SharedLock.h"
#include "utils/Iterator.h"
#include "memdbtypes.h"
#include "MapEntry.h"
#include "utils/std/Result.h"
#include "db/DbEditResult.h"

template<typename SizeValue>
class Map {
    std::atomic_uint32_t size;
    std::vector<AVLTree<SizeValue>> buckets;
    std::vector<SharedLock *> locks;
    uint32_t numberBuckets;
    friend class BucketMapHashOrderedIterator;

public:
    Map(uint32_t numberBuckets);

    /**
     * Returns true if operation was successful
     */
    std::result<DbEditResult> put(const SimpleString<SizeValue>& key,
              const SimpleString<SizeValue>& value,
              LamportClock timestamp,
              LamportClock::UpdateClockStrategy updateClockStrategy,
              lamportClock_t lamportClock,
              bool requestFromNode);

    void incrementCounter(const SimpleString<SizeValue>& key,
        uint32_t nNodes,
        memdbNodeId_t selfNodeId);

    /**
     * Returns true if operation was successful
     */
    std::result<DbEditResult> remove(const SimpleString<SizeValue>& key,
                                     LamportClock timestamp,
                                     LamportClock::UpdateClockStrategy updateClockStrategy,
                                     lamportClock_t lamportClock,
                                     bool checkTimestamps);

    std::optional<MapEntry<SizeValue>> get(const SimpleString<SizeValue>& key) const;

    void clear();
    bool contains(const SimpleString<SizeValue>& key) const;

    int getSize() const;

    static const uint8_t HASH_PRIME_FACTOR = 31;

    uint32_t calculateHash(const SimpleString<SizeValue>& key) const {
        uint32_t hashCode = 0;

        for(int i = 0; i < key.size; i++)
            hashCode += (* key[i]) * std::pow(HASH_PRIME_FACTOR, i);

        return hashCode;
    }

private:
    AVLNode<SizeValue> * getNodeByKeyHash(uint32_t keyHash) const {
        AVLTree<SizeValue> * actualMapNode = this->getBucket(keyHash);

        return actualMapNode->get(keyHash);
    }

    AVLTree<SizeValue> * getBucket(uint32_t keyHash) const {
        return const_cast<AVLTree<SizeValue> *>(this->buckets.data()) + Utils::optimizedModulePowerOfTwo(numberBuckets, keyHash);
    }

    void lockRead(uint32_t hashCode) const {
        const_cast<SharedLock *>(this->locks.at(Utils::optimizedModulePowerOfTwo(numberBuckets, hashCode)))->lockShared();
    }

    void unlockRead(uint32_t hashCode) const {
        const_cast<SharedLock *>(this->locks.at(Utils::optimizedModulePowerOfTwo(numberBuckets, hashCode)))->unlockShared();
    }

    void lockWrite(uint32_t hashCode) const {
        const_cast<SharedLock *>(this->locks.at(Utils::optimizedModulePowerOfTwo(numberBuckets, hashCode)))->lockExclusive();
    }

    void unlockWrite(uint32_t hashCode) const {
        const_cast<SharedLock *>(this->locks.at(Utils::optimizedModulePowerOfTwo(numberBuckets, hashCode)))->unlockExclusive();
    }

public:
    class BucketMapHashOrderedIterator : public Iterator<std::vector<MapEntry<memDbDataLength_t>>> {
        Map<SizeValue> * map;
        uint32_t actualBucket;

    public:
        BucketMapHashOrderedIterator(Map * map): map(map), actualBucket(0) {}

        std::vector<MapEntry<memDbDataLength_t>> next() override {
            return this->map->buckets.at(this->actualBucket++).getOrderedByHash();
        }

        bool hasNext() override {
            return this->actualBucket < this->map->numberBuckets;
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
Map<SizeValue>::Map(uint32_t numberBuckets): numberBuckets(Utils::roundUpPowerOfTwo(numberBuckets)) {
    buckets.reserve(this->numberBuckets);

    for (int i = 0; i < this->numberBuckets; i++) {
        buckets.emplace_back();
        locks.push_back(new SharedLock());
    }
}

template<typename SizeValue>
std::result<DbEditResult> Map<SizeValue>::put(const SimpleString<SizeValue> &key,
                          const SimpleString<SizeValue> &value,
                          LamportClock timestamp,
                          LamportClock::UpdateClockStrategy updateClockStrategy,
                          lamportClock_t lamportClock,
                          bool requestFromNode) {

    uint32_t keyHash = this->calculateHash(key);

    lockWrite(keyHash);

    AVLTree<SizeValue> * bucket = this->getBucket(keyHash);

    std::result<DbEditResult> addResult = bucket->addData(key, keyHash, value, timestamp, updateClockStrategy, lamportClock, requestFromNode);

    unlockWrite(keyHash);

    return addResult;
}

template<typename SizeValue>
std::optional<MapEntry<SizeValue>> Map<SizeValue>::get(const SimpleString<SizeValue>& key) const {
    uint32_t hash = this->calculateHash(key);

    lockRead(hash);

    AVLNode<SizeValue> * node = this->getNodeByKeyHash(hash);

    const std::optional<MapEntry<SizeValue>> response = (node != nullptr ?
            std::optional<MapEntry<SizeValue>>{MapEntry{node->key, node->keyHash, node->nodeType, node->data}} :
            std::nullopt);

    unlockRead(hash);

    return response;
}

template<typename SizeValue>
std::result<DbEditResult> Map<SizeValue>::remove(const SimpleString<SizeValue>& key,
                             LamportClock timestamp,
                             LamportClock::UpdateClockStrategy updateClockStrategy,
                             lamportClock_t lamportClock,
                             bool checkTimestamps) {
    uint32_t hash = this->calculateHash(key);

    lockWrite(hash);

    AVLTree<SizeValue> * bucket = this->getBucket(hash);
    std::result<DbEditResult> removeResult = bucket->remove(hash, timestamp, updateClockStrategy, lamportClock, checkTimestamps);

    unlockWrite(hash);

    return removeResult;
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