#include "gtest/gtest.h"
#include "utils/strings/SimpleString.h"

TEST(SimpleString, shouldIncreaseAndDecreaseRef) {
    SimpleString simpleString = SimpleString::fromString("hola");

    simpleString.increaseRefCount();
    simpleString.increaseRefCount();
    ASSERT_EQ(* simpleString.refCount, 3);

    simpleString.decreaseRefCount();
    simpleString.decreaseRefCount();
    ASSERT_EQ(* simpleString.refCount, 1);

    simpleString.decreaseRefCount();
    ASSERT_TRUE(simpleString.isDeleted());
}