#include <string>
#include <optional>

#include "HashCreator.h"
#include "Map.h"

class Node {
private:
    Node * next;
    int keyHash;
    char * data;
    size_t size;
    bool deleted;

public:
    Node(char * newData, int newKeyHash, size_t newSize): keyHash{newKeyHash}, data{newData}, size{newSize}, next{nullptr}, deleted{false} {}

    void replaceDeletedNode(int newHash, char * newData, size_t newSize) {
        this->data = newData;
        this->keyHash = newHash;
        this->size = newSize;
        this->deleted = false;
    }

    void replaceValue(char * newData, size_t newSize) {
        this->data = newData;
        this->size = newSize;
    }

    void setDeleted() {
        this->deleted = true;
        this->deleteDataPointer();
    }

    void setNext(Node * newNext) {
        this->next = newNext;
    }

    MapEntry toEntry() const { return MapEntry{data, size}; }
    Node * getNext() const { return this->next; }
    bool isNotDeleted() const { return !this->deleted; }
    bool isDeleted() const { return this->deleted; }
    bool hasNotSameHash(int otherHashKey) const { return this->keyHash != otherHashKey; }
    bool hasSameHash(int otherKeyHash) const { return this->keyHash == otherKeyHash; }
    bool hasNoNext() const { return this->next == nullptr; }
    bool hasNext() const { return this->next != nullptr; }

private:
    void deleteDataPointer() {
        for(int i = 0; i < this->size; i++)
            delete (this->data + i);
    }
};

class DBMapImpl : public Map {
private:
    static const int NUMBER_OF_BUCKETS = 1;

    HashCreator<std::string> * hashCreator;
    Node * buckets[NUMBER_OF_BUCKETS];
    int size;

public:
    DBMapImpl(HashCreator<std::string> * hashCreatorCons): hashCreator{hashCreatorCons}, size{0} {}

    void put(const std::string& key, char * value, size_t valueSize) override{
        int keyHash = this->hashCreator->create(key);
        Node * actualMapNode = this->getBucket(keyHash);

        while (actualMapNode->hasNext() || actualMapNode->isNotDeleted() || actualMapNode->hasNotSameHash(keyHash))
            actualMapNode = actualMapNode->getNext();

        if(actualMapNode->hasNoNext() && actualMapNode->isNotDeleted() && actualMapNode->hasNotSameHash(keyHash))
            actualMapNode->setNext( new Node(value, keyHash, valueSize));
        else if(actualMapNode->isDeleted())
            actualMapNode->replaceDeletedNode(keyHash, value, valueSize);
        else if(actualMapNode->hasSameHash(keyHash))
            actualMapNode->replaceValue(value, valueSize);
    }

    std::optional<MapEntry> get(const std::string &key) override {
        Node * nodeFoundForKey = this->getNodeByKeyHash(this->hashCreator->create(key));

        return nodeFoundForKey!= nullptr ? std::optional<MapEntry>{nodeFoundForKey->toEntry()} : std::nullopt;
    }

    std::optional<MapEntry> remove(const std::string &key) override{
        Node * nodeFoundForKey = this->getNodeByKeyHash(this->hashCreator->create(key));

        if(nodeFoundForKey)
            nodeFoundForKey->setDeleted();

        return nodeFoundForKey != nullptr ? std::optional<MapEntry>{nodeFoundForKey->toEntry()} : std::nullopt;
    }

    bool contains(const std::string &key) override {
        return this->getNodeByKeyHash(this->hashCreator->create(key)) != nullptr;
    }

    int getSize() override {
        return this->size;
    }

private:
    Node * getNodeByKeyHash(int keyHash) {
        Node * actualMapNode = this->getBucket(keyHash);

        while (actualMapNode->hasNext() || actualMapNode->isNotDeleted())
            if(actualMapNode->hasSameHash(keyHash))
                return actualMapNode;

        return actualMapNode->hasSameHash(keyHash) ? actualMapNode : nullptr;
    }

    Node * getBucket(int keyHash) {
        int bucketSlot = keyHash % NUMBER_OF_BUCKETS;
        return this->buckets[bucketSlot];
    }
};
