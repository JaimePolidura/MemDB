#pragma once

#include "vector"
#include "User.h"

class UsersService {
private:
    std::vector<User> users;

public:
    void save(int keyLength, const char * key);

    bool exists(int keyLength, const char * key);
};