#pragma once

#include <string>

struct User {
private:
    std::string authKey;

public:
    User(const std::string& authKeyCons): authKey(authKeyCons) {}

    bool doesAuthKeyMatches(const std::string& otherAuthKey) {
        return this->authKey.compare(otherAuthKey) == 0;
    }
};