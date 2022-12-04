#pragma once

#include <utility>

#include "UsersRepository.h"

class Authenticator {
private:
    UsersRepository usersRepository;

public:
    Authenticator(UsersRepository usersService): usersRepository(std::move(usersService)) {}

    bool authenticate(const std::string& otherAuthKey) {
        return this->usersRepository.existsByAuthKey(otherAuthKey);
    }
};