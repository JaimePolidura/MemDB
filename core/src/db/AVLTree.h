#pragma once

#include "shared.h"

#include "utils/strings/SimpleString.h"
#include "utils/clock/LamportClock.h"
#include "MapEntry.h"
#include "db/DbEditResult.H"
#include "utils/std/Result.h"

#define IGNORE_TIMESTAMP true
#define NOT_IGNORE_TIMESTAMP false

template<typename SizeValue>
class AVLNode {
public:
    SimpleString<SizeValue> key;
    SimpleString<SizeValue> value;
    AVLNode<SizeValue> * left;
    LamportClock timestamp;
    AVLNode<SizeValue> * right;
    uint32_t keyHash;
    int16_t height;

    bool hasNoChild() {
        return this->left == nullptr && this->right == nullptr;
    }

    AVLNode(SimpleString<SizeValue> key, uint32_t keyHash, SimpleString<SizeValue> value, int16_t height, uint16_t nodeId, uint64_t timestamp):
            left(nullptr), right(nullptr), value(value), keyHash(keyHash), height(height), key(key), timestamp(nodeId, timestamp) {
    }
};

template<typename SizeValue>
class AVLTree {
public:
    AVLNode<SizeValue> * root;

public:
    std::result<DbEditResult> add(const SimpleString<SizeValue> &key,
              uint32_t keyHash,
              const SimpleString<SizeValue> &value,
              LamportClock timestampFromNode,
              LamportClock::UpdateClockStrategy updateClockStrategy,
              lamportClock_t lamportClock) {
        AVLNode<SizeValue> * newNode = new AVLNode(key, keyHash, value, -1, 0, 0);

        DbEditResult result{};
        AVLNode<SizeValue> * insertedNode = this->insertRecursive(newNode, result, this->root, timestampFromNode, updateClockStrategy, lamportClock);

        return insertedNode != nullptr ?
            std::ok(result) :
            std::error<DbEditResult>();
    }

    std::result<DbEditResult> remove(uint32_t keyHash,
                 LamportClock timestamp,
                 LamportClock::UpdateClockStrategy updateClockStrategy,
                 lamportClock_t lamportClock) {
        bool removed = false;
        DbEditResult result{};
        this->removeRecursive(this->root, result, keyHash, timestamp, updateClockStrategy, lamportClock, removed);

        return removed ?
            std::ok(result) :
            std::error<DbEditResult>();
    }

    std::vector<AVLNode<SizeValue> *> all() const {
        std::vector<AVLNode<SizeValue> *> toReturn{};
        if(this->root == nullptr) return toReturn;

        std::queue<AVLNode<SizeValue> *> pending{};
        pending.push(this->root);

        while(!pending.empty()) {
            AVLNode<SizeValue> * node = pending.front();
            pending.pop();

            if(node->left != nullptr)
                pending.push(node->left);
            if(node->right != nullptr)
                pending.push(node->right);

            toReturn.push_back(node);
        }

        return toReturn;
    }

    bool contains(uint32_t keyHashToSearch) const {
        return this->get(keyHashToSearch) != nullptr;
    }

    AVLNode<SizeValue> * get(uint32_t keyHashToSearch) const {
        AVLNode<SizeValue> * node = this->root;

        while (node != nullptr) {
            if (node->keyHash == keyHashToSearch)
                return node;
            else if (keyHashToSearch < node->keyHash)
                node = node->left;
            else
                node = node->right;
        }

        return nullptr;
    }

    void clear(){
        this->clearRecursive(this->root);
    }

    std::vector<MapEntry<SizeValue>> getOrderedByHash() const {
        std::vector<MapEntry<SizeValue>> toReturn{};

        return this->root != nullptr ? this->getOrderedByHashRecursive(this->root, toReturn) : toReturn;
    }
    
private:
    std::vector<MapEntry<SizeValue>> getOrderedByHashRecursive(AVLNode<SizeValue> * node, std::vector<MapEntry<SizeValue>>& toReturn) const {
        if(node->left != nullptr){
            this->getOrderedByHashRecursive(node->left, toReturn);
        }
        toReturn.push_back(MapEntry<SizeValue>{node->key, node->value, node->keyHash, node->timestamp});
        if(node->right != nullptr){
            this->getOrderedByHashRecursive(node->right, toReturn);
        }

        return toReturn;
    }

    void clearRecursive(AVLNode<SizeValue> * node) {
        if(node->left != nullptr){
            this->clearRecursive(node->left);
        }
        if(node->right != nullptr){
            this->clearRecursive(node->right);
        }

        delete node;
    }

    AVLNode<SizeValue> * removeRecursive(AVLNode<SizeValue> * last,
                                         DbEditResult& result,
                                         uint32_t keyHashToRemove,
                                         LamportClock timestamp,
                                         LamportClock::UpdateClockStrategy updateClockStrategy,
                                         lamportClock_t lamportClock,
                                         bool& removed,
                                         bool alreadyDeleted = false) {

        bool ignoreTimeStamps = updateClockStrategy == LamportClock::UpdateClockStrategy::NONE;

        if(last == nullptr){
            return last;
        }else if(last->keyHash > keyHashToRemove) {
            last->left = this->removeRecursive(last->left, result, keyHashToRemove, timestamp, updateClockStrategy, lamportClock, removed, alreadyDeleted);
        }else if (last->keyHash < keyHashToRemove) {
            last->right = this->removeRecursive(last->left, result, keyHashToRemove, timestamp, updateClockStrategy, lamportClock, removed, alreadyDeleted);
        }else {
            if(!ignoreTimeStamps && last->timestamp > timestamp) {
                removed = false;
                return last;
            }

            removed = true;
            bool rootRemoved = this->root == last;

            result.timestampOfOperation = lamportClock->update(updateClockStrategy, timestamp.counter);

            if(last->left && last->right) {
                AVLNode<SizeValue> * temp = this->mostLeftChild(last->right);

                last->keyHash = temp->keyHash;
                last->value = temp->value;

                if(rootRemoved) this->root = last;

                bool ignore = false;
                last->right = removeRecursive(last->right, result, last->keyHash, last->timestamp, LamportClock::UpdateClockStrategy::NONE, lamportClock, ignore);
            }else{
                AVLNode<SizeValue> * temp = last;
                if(last->left == nullptr)
                    last = last->right;
                else if(last->right == nullptr)
                    last = last->left;

                if(rootRemoved) this->root = last;

                delete temp;
            }

            if(last != nullptr)
                last = this->rebalance(last);

            return last;
        }
    }

    AVLNode<SizeValue> * mostLeftChild(AVLNode<SizeValue> * node) const {
        while (node->left != nullptr)
            node = node->left;

        return node;
    }

    AVLNode<SizeValue> * insertRecursive(
            AVLNode<SizeValue> * toInsert,
            DbEditResult& result,
            AVLNode<SizeValue> * last,
            LamportClock timestamp,
            LamportClock::UpdateClockStrategy updateClockStrategy,
            std::shared_ptr<LamportClock> clock) {

        bool ignoreTimestamps = updateClockStrategy == LamportClock::UpdateClockStrategy::NONE;

        if(last == nullptr) {
            uint64_t newTimestampCounter = clock->update(updateClockStrategy, timestamp.getCounterValue());
            result.timestampOfOperation = newTimestampCounter;
            toInsert->timestamp.counter = newTimestampCounter;
            toInsert->timestamp.nodeId = timestamp.nodeId;

            if(this->root == nullptr){
                this->root = toInsert;
            }

            return toInsert;
        }

        if(last->keyHash > toInsert->keyHash) {
            AVLNode<SizeValue> * inserted = insertRecursive(toInsert, result, last->left, timestamp, updateClockStrategy, clock);
            if(inserted != nullptr) last->left = inserted;

        }else if(last->keyHash < toInsert->keyHash) {
            AVLNode<SizeValue> * inserted = insertRecursive(toInsert, result, last->right, timestamp, updateClockStrategy, clock);
            if(inserted != nullptr) last->right = inserted;

        }else{
            if(!ignoreTimestamps && last->timestamp > timestamp) //Reject. Node has been updated by more updated node
                return nullptr;

            last->keyHash = toInsert->keyHash;
            last->value = toInsert->value;
            last->key = toInsert->key;
            last->timestamp = LamportClock{timestamp.nodeId, clock->update(updateClockStrategy, timestamp.getCounterValue())};
        }

        return this->rebalance(last);
    }

    AVLNode<SizeValue> * rebalance(AVLNode<SizeValue> * node) {
        if(node == nullptr)
            return node;

        this->updateHeight(node);

        int16_t heightFactor = this->getHeightFactor(node);

        if(heightFactor < -1){ //Left heavy
            if(this->getHeightFactor(node->left) > 0)
                node->left = this->rotateLeft(node->left);
            node = this->rotateRight(node);
        }

        if(heightFactor > 1){ //Right heavy
            if(this->getHeightFactor(node->right) < 0)
                node->right = this->rotateRight(node->right);
            node = this->rotateLeft(node);
        }

        return node;
    }

    AVLNode<SizeValue> * rotateRight(AVLNode<SizeValue> * node) {
        AVLNode<SizeValue> * leftChild = node->left;

        this->updateRootReferenceIfNeccesary(node, leftChild);

        node->left = leftChild->right;
        leftChild->right = node;

        this->updateHeight(node);
        this->updateHeight(leftChild);

        return leftChild;
    }

    AVLNode<SizeValue> * rotateLeft(AVLNode<SizeValue> * node) {
        AVLNode<SizeValue> * rightChild = node->right;

        node->right = rightChild->left;
        rightChild->left = node;

        this->updateRootReferenceIfNeccesary(node, rightChild);

        this->updateHeight(node);
        this->updateHeight(rightChild);

        return rightChild;
    }

    void updateRootReferenceIfNeccesary(AVLNode<SizeValue> * oldReference, AVLNode<SizeValue> * newReference) {
        if(this->root == oldReference)
            this->root = newReference;
    }

    int16_t getHeightFactor(AVLNode<SizeValue> * node) const {
        return node == nullptr ? 0 : this->getHeight(node->right) - this->getHeight(node->left);
    }

    void updateHeight(AVLNode<SizeValue> * node) {
        int16_t rightHeight = this->getHeight(node->right);
        int16_t leftHeight = this->getHeight(node->left);

        node->height = std::max(leftHeight, rightHeight) + 1;
    }

    int16_t getHeight(AVLNode<SizeValue> * node) const {
        return node == nullptr ? -1 : node->height;
    }
};