#pragma once

#include <string.h>

struct User {
private:
    int keyLength;
    const char * key;

public:
    User(int keyLengthCons, const char * keyCons): keyLength(keyLengthCons), key(keyCons) {}

    bool doesKeyMatches(int otherKeyLength, const char * otherKey) {
        return otherKeyLength == this->keyLength && strcmp(this->key, otherKey) == 0;
    }
};