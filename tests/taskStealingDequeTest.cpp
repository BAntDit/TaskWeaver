//
// Created by anton on 3/19/25.
//

#include "taskStealingDequeTest.h"

TEST_F(TaskStealingDequeTest, SingleThreadedBasicOperations)
{
    // Test empty deque
    EXPECT_TRUE(deque->IsEmpty());
    EXPECT_EQ(deque->TryPop(), std::nullopt);
    EXPECT_EQ(deque->TrySteal(), std::nullopt);

    // Test TryEmplace and TryPop
    EXPECT_TRUE(deque->TryEmplace(42));
    EXPECT_FALSE(deque->IsEmpty());
    EXPECT_EQ(deque->TryPop(), 42);
    EXPECT_TRUE(deque->IsEmpty());

    // Test TryEmplace and TrySteal
    EXPECT_TRUE(deque->TryEmplace(43));
    EXPECT_EQ(deque->TrySteal(), 43);
    EXPECT_TRUE(deque->IsEmpty());
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
    EXPECT_TRUE(deque->IsEmpty());
}

TEST_F(TaskStealingDequeTest, ConcurrentTryPopAndTrySteal)
{
    constexpr auto taskCount = size_t{ 1000 };
    auto consumedTasks = std::atomic<size_t>{ 0 };

    // Producer thread
    auto producer = std::thread([this, taskCount, &consumedTasks]() {
        for (auto i = size_t{ 0 }; i < taskCount; ++i) {
            while (!deque->TryEmplace(i)) {
                std::this_thread::yield();
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });

        for (auto i = size_t{ 0 }; i < taskCount / 2; ++i) {
            auto task = std::optional<int32_t>{};
            while (!(task = deque->TryPop())) {
                std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
            }
            consumedTasks++;
        }
    });

    // Consumer threads
    auto consumer = std::thread([this, taskCount, &consumedTasks]() {
        for (auto i = size_t{ 0 }; i < taskCount / 2; ++i) {
            auto task = std::optional<int32_t>{};
            while (!(task = deque->TrySteal())) {
                std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
            }
            consumedTasks++;
        }
    });

    producer.join();
    consumer.join();

    EXPECT_EQ(consumedTasks.load(), taskCount);
    EXPECT_TRUE(deque->IsEmpty());
}

TEST_F(TaskStealingDequeTest, BottomOverflowHandling)
{
    constexpr uint64_t maxUInt64 = std::numeric_limits<uint64_t>::max();

    // Simulate bottom_ near overflow
    deque->bottom_.store(maxUInt64 - 10, std::memory_order_relaxed);
    deque->top_.store(maxUInt64 - 10, std::memory_order_relaxed);

    // Add items to the deque
    for (auto i = size_t{ 0 }; i < 20; ++i) {
        EXPECT_TRUE(deque->TryEmplace(i));
    }

    // Verify that items can be popped correctly
    for (auto i = int32_t{ 19 }; i >= 0; --i) {
        auto task = deque->TryPop();
        EXPECT_TRUE(task.has_value());
        EXPECT_EQ(task.value(), i);
    }

    EXPECT_TRUE(deque->IsEmpty());
}

TEST_F(TaskStealingDequeTest, TopOverflowHandling)
{
    constexpr uint64_t maxUInt64 = std::numeric_limits<uint64_t>::max();

    // Simulate top_ near overflow
    deque->bottom_.store(maxUInt64 - 10, std::memory_order_relaxed);
    deque->top_.store(maxUInt64 - 10, std::memory_order_relaxed);

    // Add items to the deque
    for (auto i = size_t{ 0 }; i < 20; ++i) {
        EXPECT_TRUE(deque->TryEmplace(i));
    }

    // Verify that items can be popped correctly
    for (auto i = int32_t{ 0 }; i < 20; ++i) {
        auto task = deque->TrySteal();
        EXPECT_TRUE(task.has_value());
        EXPECT_EQ(task.value(), i);
    }

    EXPECT_TRUE(deque->IsEmpty());
}

TEST_F(TaskStealingDequeTest, EdgeCases)
{
    // Test full deque
    for (auto i = size_t{ 0 }; i < deque->Capacity(); ++i) {
        EXPECT_TRUE(deque->TryEmplace(i));
    }
    EXPECT_FALSE(deque->TryEmplace(42)); // Deque should be full

    // Test popping from a full deque
    for (auto i = int32_t{ 0 }; i < static_cast<int32_t>(deque->Capacity()); ++i) {
        auto task = deque->TryPop();
        EXPECT_TRUE(task.has_value());
        EXPECT_EQ(task.value(), static_cast<int32_t>(deque->Capacity() - 1 - i));
    }
    EXPECT_TRUE(deque->IsEmpty());

    // Test stealing from an empty deque
    EXPECT_EQ(deque->TrySteal(), std::nullopt);
}
