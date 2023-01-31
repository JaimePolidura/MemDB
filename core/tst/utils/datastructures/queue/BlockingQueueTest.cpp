#include "gtest/gtest.h"
#include "utils/datastructures/queue/BlockingQueue.h"
#include <vector>

TEST(BlockingQueue, ShouldBlockAndDequeue) {
    BlockingQueue<int> queue{};

    std::thread tDequeuer([&queue]{
        int toDequeue = queue.dequeue();

        ASSERT_EQ(toDequeue, 10);
        ASSERT_TRUE(queue.getSize() == 0);
    });

    std::thread tEnqueuer([&queue]{
        std::this_thread::sleep_for(std::chrono::seconds(1));
        queue.enqueue(10);
    });

    tEnqueuer.join();
    tDequeuer.join();
}

TEST(BlockingQueue, ShouldEnqueueAndDequeue) {
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

TEST(BlockingQueue, SizeShouldBeZero) {
    BlockingQueue<int> queue{};

    ASSERT_EQ(queue.getSize(), 0);
}