#include "gtest/gtest.h"
#include "utils/datastructures/queue/BlockingQueue.h"
#include <memory>
#include <vector>

TEST(LockFreeQueue, ShouldEnqueueAndDequeue) {
    BlockingQueue<int> queue{};

    queue.enqueue(1);
    ASSERT_EQ(queue.getSize(), 1);

    queue.enqueue(2);
    ASSERT_EQ(queue.getSize(), 2);

    queue.enqueue(3);
    ASSERT_EQ(queue.getSize(), 3);

    ASSERT_EQ(queue.dequeue(), 1);
    ASSERT_EQ(queue.getSize(), 2);

    ASSERT_EQ(queue.dequeue(), 2);
    ASSERT_EQ(queue.getSize(), 1);

    ASSERT_EQ(queue.dequeue(), 3);
    ASSERT_EQ(queue.getSize(), 0);
}

TEST(LockFreeQueue, SizeShouldBeZero) {
    BlockingQueue<int> queue{};

    ASSERT_EQ(queue.getSize(), 0);
}