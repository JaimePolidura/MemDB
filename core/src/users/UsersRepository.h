#pragma once

#include "vector"
#include "User.h"

class UsersRepository {
private:
    std::vector<User> users;

public:
    void save(const User& user);

    bool existsByAuthKey(const std::string& otherAuthkey);
};