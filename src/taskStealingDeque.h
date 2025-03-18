//
// Created by anton on 3/15/25.
//

#ifndef TASKWEAVER_TASKSTEALINGDEQUE_H
#define TASKWEAVER_TASKSTEALINGDEQUE_H

#include "common.h"
#include <cassert>
#include <optional>

namespace taskweaver {
template<typename T>
concept DequeItemConcept = (std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T> &&
                            std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>);

// circular work-stealing deque, SPMC and lock free
// the same idea as in paper below, but with static size ring
// https://www.dre.vanderbilt.edu/~schmidt/PDF/work-stealing-dequeue.pdf
template<DequeItemConcept T>
class TaskStealingDeque
{
    struct data_t
    {
        explicit data_t(size_t capacity);

        [[nodiscard]] auto capacity() const -> size_t { return capacity_; }

        void store(size_t index, T&& t) noexcept;

        auto load(size_t index) noexcept -> T;

    private:
        size_t capacity_;
        std::unique_ptr<T[]> data_;
    };

public:
    explicit TaskStealingDeque(size_t capacity);

    ~TaskStealingDeque();

    [[nodiscard]] auto Capacity() const -> size_t { return data_->capacity(); }

    [[nodiscard]] auto Empty() const -> bool;

    // can be called in producer thread only
    template<typename... Args>
    auto TryEmplace(Args&&... args) -> bool;

    // can be called in any thread, but
    // it's maden in purpose of consumer threads
    // in producer thread better use TryPop method instead
    auto TrySteal() -> std::optional<T>;

    // can be called in producer thread only
    auto TryPop() -> std::optional<T>;

private:
    [[nodiscard]] auto CountItems() const -> size_t;

    alignas(hardware_destructive_interference_size) std::atomic<uint64_t> top_;
    alignas(hardware_destructive_interference_size) std::atomic<uint64_t> bottom_;
    alignas(hardware_destructive_interference_size) data_t* data_;
};

template<DequeItemConcept T>
TaskStealingDeque<T>::data_t::data_t(size_t capacity)
  : capacity_{ capacity }
  , data_{ make_unique_for_overwrite<T[]>(capacity) }
{
}

template<DequeItemConcept T>
void TaskStealingDeque<T>::data_t::store(size_t index, T&& t) noexcept
{
    data_[index % capacity_] = std::move(t);
}

template<DequeItemConcept T>
auto TaskStealingDeque<T>::data_t::load(size_t index) noexcept -> T
{
    return data_[index % capacity_];
}

template<DequeItemConcept T>
TaskStealingDeque<T>::TaskStealingDeque(size_t capacity)
  : top_{ 0 }
  , bottom_{ 0 }
  , data_{ new data_t{ capacity } }
{
}

template<DequeItemConcept T>
TaskStealingDeque<T>::~TaskStealingDeque()
{
    delete data_;
}

template<DequeItemConcept T>
auto TaskStealingDeque<T>::Empty() const -> bool
{
    auto bottom = bottom_.load(std::memory_order_acquire);
    auto top = top_.load(std::memory_order_acquire);

    return top != bottom;
}

template<DequeItemConcept T>
auto TaskStealingDeque<T>::CountItems() const -> size_t
{
    auto bottom = bottom_.load(std::memory_order_relaxed);
    auto top = top_.load(std::memory_order_acquire);

    return static_cast<size_t>((bottom >= top) ? bottom - top : (Capacity() - top + bottom));
}

template<DequeItemConcept T>
template<typename... Args>
auto TaskStealingDeque<T>::TryEmplace(Args&&... args) -> bool
{
    if (CountItems() < Capacity()) {
        data_->store(static_cast<size_t>(bottom_.load(std::memory_order_relaxed)), T(std::forward<Args>(args)...));
        bottom_.fetch_add(1, std::memory_order_relaxed);
        return true;
    }

    return false;
}

template<DequeItemConcept T>
auto TaskStealingDeque<T>::TrySteal() -> std::optional<T>
{
    auto result = std::optional<T>{ std::nullopt };

    auto top = top_.load(std::memory_order_acquire);

    std::atomic_thread_fence(std::memory_order_release);
    auto bottom = bottom_.load(std::memory_order_acquire);

    if (top != bottom) {
        result = data_->load(static_cast<size_t>(top));
    }

    if (!top_.compare_exchange_strong(top, top + 1, std::memory_order_acq_rel, std::memory_order_relaxed)) {
        result = std::nullopt;
    }

    return result;
}

template<DequeItemConcept T>
auto TaskStealingDeque<T>::TryPop() -> std::optional<T>
{
    auto result = std::optional<T>{ std::nullopt };

    auto bottom = bottom_.load(std::memory_order_relaxed);
    auto top = top_.load(std::memory_order_acquire);

    if (bottom != top) { // not empty
        auto reserved = bottom--;
        bottom_.store(bottom, std::memory_order_release);

        if (reserved == top_.load(std::memory_order_acquire)) { // someone stole everything
            bottom_.store(bottom + 1, std::memory_order_relaxed);
        } else {
            result = data_->load(static_cast<size_t>(reserved));
        }
    }

    return result;
}
}

#endif // TASKWEAVER_TASKSTEALINGDEQUE_H
