//
// Created by anton on 3/22/25.
//

#include "taskManagerTest.h"
#include "../src/utility.h"
#include <cmath>

namespace {
struct alignas(hardware_constructive_interference_size) data_item_t
{
    uint32_t val;
};

template<size_t N>
auto prefix_sum(std::array<data_item_t, N>& src, size_t idx, size_t step) -> data_item_t
{
    auto sh = static_cast<size_t>(std::pow(2, step));
    auto idx0 = idx;
    auto idx1 = (sh <= idx) ? (idx - sh) : std::numeric_limits<size_t>::max();

    auto a = src[idx0].val;
    auto b = (idx1 < N) ? src[idx1].val : 0;

    return data_item_t{ a + b };
}

template<size_t N, size_t... I>
auto schedule_tasks(std::array<data_item_t, N>& src, size_t step, std::index_sequence<I...>)
{
    return taskweaver::when_all(taskweaver::TaskManager::SubmitTask(
      [&src, step, idx = I]() -> data_item_t { return prefix_sum(src, idx, step); })...);
}

template<typename Tuple, size_t N, size_t... I>
void tuple_to_array(Tuple&& src, std::array<data_item_t, N>& dst, std::index_sequence<I...>)
{
    ((dst[I] = std::get<I>(src)), ...);
}
}

void parrallelComputationWhenAllTest()
{
    auto src = std::array<data_item_t, 8>{ 3, 1, 4, 1, 5, 9, 2, 6 };

    auto dst1 = std::array<data_item_t, 8>{};
    auto dst2 = std::array<data_item_t, 8>{};

    constexpr auto size = std::size(src);
    auto steps = static_cast<size_t>(std::log(size));

    // inclusive scan
    // consecutive
    {
        auto tmp = src;
        for (auto i = size_t{ 0 }; i <= steps; i++) {
            for (auto j = size_t{ 0 }; j < size; j++) {
                dst1[j] = prefix_sum(tmp, j, i);
            }

            if (i < steps)
                std::swap(tmp, dst1);
        }
    }

    // parallel
    {
        auto tmp = src;

        for (auto i = size_t{ 0 }; i <= steps; i++) {
            tuple_to_array(
              schedule_tasks(tmp, i, std::make_index_sequence<size>{}).get(), dst2, std::make_index_sequence<size>{});

            if (i < steps)
                std::swap(tmp, dst2);
        }
    }

    // test (compare consecutive and parallel)
    for (auto i = size_t{ 0 }; i < size; i++) {
        ASSERT_EQ(dst1[i].val, dst2[i].val);
    }
}

void whenAnyTest() {
    auto futures = std::array{ taskweaver::TaskManager::SubmitTask([]() -> uint32_t {
        std::this_thread::sleep_for(std::chrono::seconds{ 2 });
        return 2;
    }), taskweaver::TaskManager::SubmitTask([]() -> uint32_t {
        std::this_thread::sleep_for(std::chrono::seconds{ 1 });
        return 1;
    }) };

    auto res = taskweaver::when_any(futures.begin(), futures.end()).get();

    auto b = (res.result == 2 && res.index == 0) || (res.result == 1 && res.index == 1);

    EXPECT_TRUE(b);
}

TEST_F(TaskManagerTest, ParrallelComputationWhenAll)
{
    parrallelComputationWhenAllTest();
}

TEST_F(TaskManagerTest, WhenAny)
{
    whenAnyTest();
}
