#include <string>
#include <optional>

#include "HashCreator.h"
#include "Map.h"
#include "DBMap.h"

DBMap::DBMap(HashCreator<std::string> * hashCreatorCons): hashCreator{hashCreatorCons}, size{0} {}

void DBMap::put(const std::string &key, char * value, size_t valueSize) {
    int keyHash = this->hashCreator->create(key);
    Node * actualMapNode = this->getBucket(keyHash);

    while (actualMapNode != nullptr && (actualMapNode->hasNext() || actualMapNode->isNotDeleted() ||actualMapNode->hasNotSameHash(keyHash)))
        actualMapNode = actualMapNode->getNext();

    if(actualMapNode == nullptr)
        this->buckets[this->getBucketSlot(keyHash)] = new Node(value, keyHash, valueSize);
    else if(actualMapNode->hasNoNext() && actualMapNode->isNotDeleted() && actualMapNode->hasNotSameHash(keyHash))
        actualMapNode->setNext( new Node(value, keyHash, valueSize));
    else if(actualMapNode->isDeleted())
        actualMapNode->replaceDeletedNode(keyHash, value, valueSize);
    else if(actualMapNode->hasSameHash(keyHash))
        actualMapNode->replaceValue(value, valueSize);

    this->size++;
}

std::optional<MapEntry> DBMap::get(const std::string &key) {
    Node * nodeFoundForKey = this->getNodeByKeyHash(this->hashCreator->create(key));

    return nodeFoundForKey!= nullptr ? std::optional<MapEntry>{nodeFoundForKey->toEntry()} : std::nullopt;
}

std::optional<MapEntry> DBMap::remove(const std::string &key) {
    Node * nodeFoundForKey = this->getNodeByKeyHash(this->hashCreator->create(key));

    if(nodeFoundForKey)
        nodeFoundForKey->setDeleted();

    return nodeFoundForKey != nullptr ? std::optional<MapEntry>{nodeFoundForKey->toEntry()} : std::nullopt;
}

bool DBMap::contains(const std::string &key) {
    return this->getNodeByKeyHash(this->hashCreator->create(key)) != nullptr;
}

int DBMap::getSize() {
    return this->size;
}