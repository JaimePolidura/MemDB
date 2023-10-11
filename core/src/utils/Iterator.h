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