#pragma once

template<typename T>
class HashCreator {
public:
    virtual int create(const T &toHash) = 0;
};