#include "gtest/gtest.h"
#include "users/UsersRepository.h"

TEST(UsersService, ShouldSaveAndGet) {
    UsersRepository usersService{};
    const char * userKey = "hola";
    usersService.save(User{userKey});

    ASSERT_FALSE(usersService.existsByAuthKey("prueba"));
    ASSERT_FALSE(usersService.existsByAuthKey("hold"));
    ASSERT_TRUE(usersService.existsByAuthKey("hola"));
}