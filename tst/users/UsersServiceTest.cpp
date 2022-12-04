#include "gtest/gtest.h"
#include "users/UsersRepository.h"

TEST(UsersService, ShouldSaveAndGet) {
    UsersRepository usersService{};
    const char * userKey = "hola";
    usersService.save(4, userKey);

    ASSERT_FALSE(usersService.exists(5, "prueba"));
    ASSERT_FALSE(usersService.exists(4, "hold"));
    ASSERT_TRUE(usersService.exists(4, "hola"));
}