//
// Created by anton on 3/19/25.
//

#include "taskStealingDequeTest.h"

TEST_F(TaskStealingDequeTest, SingleThreadedBasicOperations)
{
    // Test empty deque
    EXPECT_TRUE(deque->Empty());
    EXPECT_EQ(deque->TryPop(), std::nullopt);
    EXPECT_EQ(deque->TrySteal(), std::nullopt);

    // Test TryEmplace and TryPop
    EXPECT_TRUE(deque->TryEmplace(42));
    EXPECT_FALSE(deque->Empty());
    EXPECT_EQ(deque->TryPop(), 42);
    EXPECT_TRUE(deque->Empty());

    // Test TryEmplace and TrySteal
    EXPECT_TRUE(deque->TryEmplace(43));
    EXPECT_EQ(deque->TrySteal(), 43);
    EXPECT_TRUE(deque->Empty());
}

TEST_F(TaskStealingDequeTest, ConcurrentProducerConsumer)
{
    constexpr auto taskCount = size_t{ 1000 };
    auto consumedTasks = std::atomic<size_t>{ 0 };

    // Producer thread
    auto producer = std::thread([=, this]() {
        for (auto i = size_t{ 0 }; i < taskCount; ++i) {
            while (!deque->TryEmplace(static_cast<int32_t>(i))) {
                std::this_thread::yield();
            }
        }
    });

    // Consumer thread
    auto consumer = std::thread([this, taskCount, &consumedTasks]() {
        for (auto i = size_t{ 0 }; i < taskCount; ++i) {
            auto task = std::optional<int32_t>{};
            while (!(task = deque->TrySteal())) {
                std::this_thread::yield();
            }
            consumedTasks++;
        }
    });

    producer.join();
    consumer.join();

    EXPECT_EQ(consumedTasks.load(), taskCount);
    EXPECT_TRUE(deque->Empty());
}

TEST_F(TaskStealingDequeTest, ConcurrentTryPopAndTrySteal) {
    constexpr auto taskCount = size_t{ 1000 };
    auto consumedTasks = std::atomic<size_t>{ 0 };

    // TODO::
}
