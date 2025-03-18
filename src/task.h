//
// Created by anton on 3/18/25.
//

#ifndef TASKWEAVER_TASK_H
#define TASKWEAVER_TASK_H

#include "common.h"
#include <cassert>

namespace taskweaver {
class Task;

template<typename T>
concept TaskFunctorConcept = !
std::is_same_v<std::decay_t<T>, Task>;

class alignas(hardware_constructive_interference_size) Task
{
    static constexpr size_t storage_align_v = alignof(void (*)());
    static constexpr size_t storage_size_v = 64;

    struct functor_base_t
    {
        functor_base_t() = default;

        virtual ~functor_base_t() = default;

        virtual void invoke() = 0;

        virtual auto move_to(std::byte (&storage)[storage_size_v]) -> functor_base_t* = 0;
    };

    template<typename F>
    struct functor_t : functor_base_t
    {
        explicit functor_t(F const& f)
          : f_(f)
        {
        }

        explicit functor_t(F&& f)
          : f_(std::move(f))
        {
        }

        void invoke() override { f_(); }

        auto move_to(std::byte (&storage)[storage_size_v]) -> functor_base_t* override;

    private:
        F f_;
    };

public:
    Task();

    template<TaskFunctorConcept F>
    explicit Task(F&& f);

    Task(Task const&) = delete;

    Task(Task&& task) noexcept;

    auto operator=(Task const&) -> Task& = delete;

    auto operator=(Task&& rhs) noexcept -> Task&;

    void operator()();

    [[nodiscard]] auto pending() const -> bool { return pending_.load(std::memory_order_acquire); }

    ~Task();

private:
    void _reset();

    auto storage() -> void* { return storage_; }

private:
    alignas(storage_align_v) std::byte storage_[storage_size_v]; // for small functor optimization (sfo)
    functor_base_t* functor_;
    std::atomic<bool> pending_;
};

template<typename F>
auto Task::functor_t<F>::move_to(std::byte (&storage)[storage_size_v]) -> functor_base_t*
{
    auto* r = std::add_pointer_t<functor_base_t>{ nullptr };

    void* sdata = std::data(storage);
    size_t ssize = std::size(storage);

    if (auto* p = std::align(alignof(functor_t<F>), sizeof(functor_t<F>), sdata, ssize); p == std::data(storage)) {
        r = new (p) functor_t<F>{ std::move(f_) };
    }

    assert(r != nullptr);
    return r;
}

template<TaskFunctorConcept F>
Task::Task(F&& f)
  : storage_{}
  , functor_{ nullptr }
  , pending_{ false }
{
    void* storage = std::data(storage_);
    auto size = std::size(storage_);

    if (auto* p = std::align(alignof(functor_t<F>), sizeof(functor_t<F>), storage, size); p == std::data(storage_)) {
        // in case functor is small enough and functor alignment matches with sfo buffer alignment
        functor_ = new (p) functor_t<F>{ std::forward<F>(f) };
    } else {
        functor_ = new functor_t<F>{ std::forward<F>(f) };
    }
    pending_.store(functor_ != nullptr, std::memory_order_relaxed);
}
}

#endif // TASKWEAVER_TASK_H
