#include "Map.h"

Map::Map(uint16_t numberBuckets): numberBuckets(numberBuckets) {
    printf("Hi\n");

    buckets.reserve(numberBuckets);

    for (int i = 0; i < numberBuckets; i++)
        buckets.emplace_back();
}

void Map::put(const std::string &key, uint8_t * value, size_t valueSize) {
    uint32_t keyHash = this->calculateHash(key);
    AVLTree bucket = this->getBucket(keyHash);

    bucket.add(keyHash, value, valueSize);

    this->size++;
}

std::optional<MapEntry> Map::get(const std::string &key) const {
    AVLNode * nodeFoundForKey = this->getNodeByKeyHash(this->calculateHash(key));

    return nodeFoundForKey != nullptr ? std::optional<MapEntry>{MapEntry{nodeFoundForKey->value, nodeFoundForKey->valueLength}} : std::nullopt;
}

void Map::remove(const std::string &key) {
    uint32_t hash = this->calculateHash(key);
    AVLNode * nodeFoundForKey = this->getNodeByKeyHash(hash);

    if(nodeFoundForKey != nullptr) {
        AVLTree bucket = this->getBucket(hash);
        bucket.remove(hash);
        this->size--;
    }
}

bool Map::contains(const std::string &key) const {
    return this->getNodeByKeyHash(this->calculateHash(key)) != nullptr;
}

int Map::getSize() const {
    return this->size;
}