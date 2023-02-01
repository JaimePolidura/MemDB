#include "gtest/gtest.h"
#include "utils/threads/ReadWriteLock.h"

TEST(ReadWriteLock, ShouldntBlock) {
    ReadWriteLock lock{};

    lock.lockRead();
    lock.lockRead();
    lock.unlockRead();
    lock.unlockRead();

    lock.lockWrite();
    lock.unlockWrite();

    lock.lockRead();
}