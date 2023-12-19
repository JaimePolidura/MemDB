#pragma once

#include "shared.h"

#include "utils/strings/SimpleString.h"
#include "utils/clock/LamportClock.h"
#include "MapEntry.h"
#include "db/DbEditResult.h"
#include "utils/std/Result.h"
#include "db/counters/UpdateCounterType.h"
#include "counters/Counter.h"
#include "db/NodeType.h"

#define IGNORE_TIMESTAMP true
#define NOT_IGNORE_TIMESTAMP false

template<typename SizeValue>
class AVLNode {
public:
    SimpleString<SizeValue> key;
    AVLNode<SizeValue> * left;
    AVLNode<SizeValue> * right;
    std::shared_ptr<void> data;
    uint32_t keyHash;
    int16_t height;
    NodeType nodeType;

    bool hasNoChild() {
        return this->left == nullptr && this->right == nullptr;
    }

    AVLNode(SimpleString<SizeValue> key, uint32_t keyHash, int16_t height, NodeType nodeType, std::shared_ptr<void> data) :
            left(nullptr), right(nullptr), keyHash(keyHash), height(height), key(key), nodeType(nodeType), data(data) {
    }
};

template<typename SizeValue>
class AVLTree {
public:
    AVLNode<SizeValue> * root;

    std::result<DbEditResult> addData(const SimpleString<SizeValue> &key,
              uint32_t keyHash,
              const SimpleString<SizeValue> &value,
              LamportClock timestampFromNode,
              LamportClock::UpdateClockStrategy updateClockStrategy,
              lamportClock_t lamportClock,
              bool requestFromNode) {
        AVLNode<SizeValue> * newDataNode = new AVLNode(key, keyHash, -1, NodeType::DATA, std::make_shared<DataAVLNode<SizeValue>>(value, LamportClock{0, 0}));

        DbEditResult result{.timestampOfOperation = 0};
        AVLNode<SizeValue> * insertedNode = this->insertRecursiveData(newDataNode, result, this->root, timestampFromNode, updateClockStrategy, lamportClock, requestFromNode);

        return insertedNode != nullptr ?
            std::ok(result) :
            std::error<DbEditResult>();
    }

    std::result<DbEditResult> removeData(uint32_t keyHash,
                 LamportClock timestamp,
                 LamportClock::UpdateClockStrategy updateClockStrategy,
                 lamportClock_t lamportClock,
                 bool checkTimestamps) {
        bool removed = false;
        DbEditResult result{};
        this->removeRecursiveData(this->root, result, keyHash, timestamp, updateClockStrategy, lamportClock, removed, checkTimestamps);

        return removed ?
            std::ok(result) :
            std::error<DbEditResult>();
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
        toReturn.push_back(MapEntry<SizeValue>{node->key, node->keyHash, node->nodeType, node->data});
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

    AVLNode<SizeValue> * removeRecursiveData(AVLNode<SizeValue> * last,
                                         DbEditResult& result,
                                         uint32_t keyHashToRemove,
                                         LamportClock timestamp,
                                         LamportClock::UpdateClockStrategy updateClockStrategy,
                                         lamportClock_t lamportClock,
                                         bool& removed,
                                         bool checkTimestamps) {

        bool ignoreTimeStamps = updateClockStrategy == LamportClock::UpdateClockStrategy::NONE || !checkTimestamps;

        if(last == nullptr){
            return last;
        }else if(last->keyHash > keyHashToRemove) {
            last->left = this->removeRecursiveData(last->left, result, keyHashToRemove, timestamp, updateClockStrategy, lamportClock, removed, checkTimestamps);
        }else if (last->keyHash < keyHashToRemove) {
            last->right = this->removeRecursiveData(last->left, result, keyHashToRemove, timestamp, updateClockStrategy, lamportClock, removed, checkTimestamps);
        }else {
            if(last->nodeType != NodeType::DATA) {
                result.timestampOfOperation = 0;
                removed = false;
                return last;
            }

            DataAVLNode<SizeValue> * dataNodeLast = std::dynamic_pointer_cast<DataAVLNode<SizeValue>>(last->data).get();

            if(!ignoreTimeStamps && dataNodeLast->timestamp > timestamp) {
                result.timestampOfOperation = 0;
                removed = false;
                return last;
            }

            removed = true;
            bool rootRemoved = this->root == last;
            result.timestampOfOperation = lamportClock->update(updateClockStrategy, timestamp.counter);

            if(last->left && last->right) {
                AVLNode<SizeValue> * temp = this->mostLeftChild(last->right);

                //TODO Review
                last->keyHash = temp->keyHash;
                last->data = temp->data;
                last->nodeType = temp->data;

                if(rootRemoved) this->root = last;

                bool ignore = false;
                last->right = removeRecursive(last->right, result, last->keyHash, dataNodeLast->timestamp, LamportClock::UpdateClockStrategy::NONE, lamportClock, ignore, ignoreTimeStamps);
            }else{
                AVLNode<SizeValue> * temp = last;
                if(last->left == nullptr)
                    last = last->right;
                else if(last->right == nullptr)
                    last = last->left;

                if(rootRemoved) this->root = last;

                delete temp;
            }
        }

        if(last != nullptr)
            last = this->rebalance(last);

        return last;
    }

    AVLNode<SizeValue> * mostLeftChild(AVLNode<SizeValue> * node) const {
        while (node->left != nullptr)
            node = node->left;

        return node;
    }

    AVLNode<SizeValue> * insertRecursiveData(
            AVLNode<SizeValue> * toInsert,
            DbEditResult& result,
            AVLNode<SizeValue> * last,
            LamportClock timestamp,
            LamportClock::UpdateClockStrategy updateClockStrategy,
            std::shared_ptr<LamportClock> clock,
            bool requestFromNode) {

        bool ignoreTimestamps = updateClockStrategy == LamportClock::UpdateClockStrategy::NONE || !requestFromNode;

        if(last == nullptr) {
            uint64_t newTimestampCounter = this->updateClock(clock, updateClockStrategy, timestamp.getCounterValue());

            DataAVLNode<SizeValue> * dataNodeToInsert = std::static_pointer_cast<DataAVLNode<SizeValue>>(toInsert->data).get();

            if(requestFromNode){
                result.timestampOfOperation = timestamp.counter;
                dataNodeToInsert->timestamp = timestamp;
            } else {
                result.timestampOfOperation = newTimestampCounter;
                dataNodeToInsert->timestamp.counter = newTimestampCounter;
                dataNodeToInsert->timestamp.nodeId = timestamp.nodeId;
            }

            if(this->root == nullptr){
                this->root = toInsert;
            }

            return toInsert;
        }

        if(last->keyHash > toInsert->keyHash) {
            AVLNode<SizeValue> * inserted = insertRecursiveData(toInsert, result, last->left, timestamp, updateClockStrategy, clock, requestFromNode);
            if(inserted != nullptr) last->left = inserted;

        }else if(last->keyHash < toInsert->keyHash) {
            AVLNode<SizeValue> * inserted = insertRecursiveData(toInsert, result, last->right, timestamp, updateClockStrategy, clock, requestFromNode);
            if(inserted != nullptr) last->right = inserted;

        }else{
            if(last->nodeType != NodeType::DATA) {
                return nullptr; //Error
            }

            DataAVLNode<SizeValue> * dataNodeToInsert = std::static_pointer_cast<DataAVLNode<SizeValue>>(toInsert->data).get();
            DataAVLNode<SizeValue> * dataLastNode = std::static_pointer_cast<DataAVLNode<SizeValue>>(last->data).get();

            if(!ignoreTimestamps && dataLastNode->timestamp > timestamp) //Reject. Node has been updated by more updated node
                return nullptr;

            dataLastNode->value = dataNodeToInsert->value;

            uint64_t newTimestampCounter = clock->update(updateClockStrategy, timestamp.getCounterValue());
            if(requestFromNode) {
                result.timestampOfOperation = timestamp.counter;
                dataLastNode->timestamp = timestamp;
            } else {
                result.timestampOfOperation = newTimestampCounter;
                dataLastNode->timestamp.counter = newTimestampCounter;
                dataLastNode->timestamp.nodeId = timestamp.nodeId;
            }
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

    uint64_t updateClock(lamportClock_t lamportClock, LamportClock::UpdateClockStrategy updateType, uint64_t othersValue) {
        if(lamportClock != nullptr) {
            return lamportClock->update(updateType, othersValue);
        } else {
            return 0;
        }
    }
};