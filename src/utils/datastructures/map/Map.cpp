#include "Map.h"

Map::Map(): size(0) {}

void Map::put(const std::string &key, uint8_t * value, size_t valueSize) {
    int keyHash = this->calculateHash(key);
    MapNode * actualMapNode = this->getBucket(keyHash);

    while (actualMapNode != nullptr && (actualMapNode->hasNext() || actualMapNode->isNotDeleted() ||actualMapNode->hasNotSameHash(keyHash)))
        actualMapNode = actualMapNode->getNext();
    if(actualMapNode == nullptr)
        this->buckets[this->getBucketSlot(keyHash)] = new MapNode(value, keyHash, valueSize);
    else if(actualMapNode->hasNoNext() && actualMapNode->isNotDeleted() && actualMapNode->hasNotSameHash(keyHash))
        actualMapNode->setNext( new MapNode(value, keyHash, valueSize));
    else if(actualMapNode->isDeleted())
        actualMapNode->replaceDeletedNode(keyHash, value, valueSize);
    else if(actualMapNode->hasSameHash(keyHash))
        actualMapNode->replaceValue(value, valueSize);

    this->size++;
}

std::optional<MapEntry> Map::get(const std::string &key) const {
    MapNode * nodeFoundForKey = this->getNodeByKeyHash(this->calculateHash(key));

    return nodeFoundForKey != nullptr ? std::optional<MapEntry>{nodeFoundForKey->toEntry()} : std::nullopt;
}

void Map::remove(const std::string &key) {
    MapNode * nodeFoundForKey = this->getNodeByKeyHash(this->calculateHash(key));

    if(nodeFoundForKey)
        nodeFoundForKey->setDeleted();
}

bool Map::contains(const std::string &key) const {
    return this->getNodeByKeyHash(this->calculateHash(key)) != nullptr;
}

int Map::getSize() const {
    return this->size;
}