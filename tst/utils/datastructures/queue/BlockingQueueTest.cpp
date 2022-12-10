#include "gtest/gtest.h"
#include "utils/datastructures/queue/BlockingQueue.h"
#include <memory>

TEST(LockFreeQueue, SizeShouldBeZero) {
    BlockingQueue<int> queue{};

    ASSERT_EQ(queue.getSize(), 0);
}