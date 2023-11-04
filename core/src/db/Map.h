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
private:
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
              LamportClock timestampFromNode,
              LamportClock::UpdateClockStrategy updateClockStrategy,
              lamportClock_t lamportClock);

    /**
     * Returns true if operation was successful
     */
    std::result<DbEditResult> remove(const SimpleString<SizeValue>& key,
                                     LamportClock timestamp,
                                     LamportClock::UpdateClockStrategy updateClockStrategy,
                                     lamportClock_t lamportClock);


    std::optional<MapEntry<SizeValue>> get(const SimpleString<SizeValue>& key) const;

    void clear();
    bool contains(const SimpleString<SizeValue>& key) const;

    std::vector<MapEntry<SizeValue>> all();

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
    private:
        Map<SizeValue> * map;
        uint32_t actualBucket;

    public:
        BucketMapHashOrderedIterator(Map * map): map(map), actualBucket(0) {}

        std::vector<MapEntry<memDbDataLength_t>> next() override {
            return this->map->buckets.at(this->actualBucket++).getOrderedByHash();;
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
                          LamportClock timestampFromNode,
                          LamportClock::UpdateClockStrategy updateClockStrategy,
                          lamportClock_t lamportClock) {

    uint32_t keyHash = this->calculateHash(key);

    lockWrite(keyHash);

    AVLTree<SizeValue> * bucket = this->getBucket(keyHash);

    std::result<DbEditResult> addResult = bucket->add(key, keyHash, value, timestampFromNode, updateClockStrategy, lamportClock);

    unlockWrite(keyHash);

    return addResult;
}

template<typename SizeValue>
std::vector<MapEntry<SizeValue>> Map<SizeValue>::all() {
    std::vector<MapEntry<SizeValue>> all{};

    for (const AVLTree bucket: this->buckets){
        for (const auto node : bucket.all()){
            all.push_back(MapEntry{.key = node->key, .value = node->_value, .keyHash = node->keyHash, .timestamp = node->timestamp});
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
            std::optional<MapEntry<SizeValue>>{MapEntry{node->key, node->value, node->keyHash, node->timestamp}} :
            std::nullopt);

    unlockRead(hash);

    return response;
}

template<typename SizeValue>
std::result<DbEditResult> Map<SizeValue>::remove(const SimpleString<SizeValue>& key,
                             LamportClock timestamp,
                             LamportClock::UpdateClockStrategy updateClockStrategy,
                             lamportClock_t lamportClock) {
    uint32_t hash = this->calculateHash(key);

    lockWrite(hash);

    AVLTree<SizeValue> * bucket = this->getBucket(hash);
    std::result<DbEditResult> removeResult = bucket->remove(hash, timestamp, updateClockStrategy, lamportClock);

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