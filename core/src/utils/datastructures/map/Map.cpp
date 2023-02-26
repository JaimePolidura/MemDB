#include "Map.h"

template<typename SizeValue>
Map<SizeValue>::Map(uint16_t numberBuckets): numberBuckets(numberBuckets) {
    buckets.reserve(numberBuckets);

    for (int i = 0; i < numberBuckets; i++) {
        buckets.emplace_back();
        locks.push_back(new ReadWriteLock());
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