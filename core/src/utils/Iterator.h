#pragma once

#include "shared.h"

template<typename T>
class Iterator {
public:
    virtual bool hasNext() = 0;
    virtual T next() = 0;
    virtual uint64_t totalSize() = 0;
};

template<typename T>
using iterator_t = std::shared_ptr<Iterator<T>>;

template<typename T>
class NullIterator : public Iterator<T> {
public:
    bool hasNext() override {
        return false;
    }
    
    T next() override {
        throw std::runtime_error("Iterator is emtpy. Execute hasNext() first");
    }

    uint64_t totalSize() override {
        return 0;
    }
};