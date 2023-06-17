#include "gtest/gtest.h"
#include "utils/threads/SharedLock.h"

TEST(ReadWriteLock, ShouldntBlock) {
    SharedLock lock{};

    lock.lockShared();
    lock.lockShared();
    lock.unlockShared();
    lock.unlockShared();

    lock.lockExclusive();
    lock.unlockExclusive();

    lock.lockShared();
}