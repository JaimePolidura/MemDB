#pragma once

#include <utility>
#include <memory>

#include "UsersRepository.h"

class Authenticator {
private:
    std::shared_ptr<UsersRepository> usersRepository;

public:
    Authenticator() = default;

    Authenticator(std::shared_ptr<UsersRepository> usersRepository): usersRepository(usersRepository) {}

    bool authenticate(const std::string& otherAuthKey) {
        return this->usersRepository->existsByAuthKey(otherAuthKey);
    }
};