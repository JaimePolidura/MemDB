#include "UsersRepository.h"

#include "User.h"

void UsersRepository::save(const User& user) {
    this->users.emplace_back(user);
}

bool UsersRepository::existsByAuthKey(const std::string& otherAuthkey) {
    for(int i = 0; i < this->users.size(); i++)
        if(this->users[i].doesAuthKeyMatches(otherAuthkey))
            return true;

    return false;
}
