#include "UsersService.h"

#include "User.h"

void UsersService::save(int keyLength, const char * key) {
    //TODO Optimize. See move constructors, move semantics etc.
    this->users.emplace_back(User{keyLength, key});
}

bool UsersService::exists(int keyLength, const char * key) {
    for(int i = 0; i < this->users.size(); i++)
        if(this->users[i].doesKeyMatches(keyLength, key))
            return true;

    return false;
}
