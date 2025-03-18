//
// Created by anton on 3/18/25.
//

#include "task.h"
#include <utility>

namespace taskweaver {
Task::Task()
  : storage_{}
  , functor_{ nullptr }
  , pending_{ false }
{
}

Task::Task(Task&& task) noexcept
  : storage_{}
  , functor_{ nullptr }
  , pending_{ false }
{
    assert(task.functor_);

    if (task.storage_ == reinterpret_cast<std::byte*>(task.functor_)) {
        functor_ = task.functor_->move_to(storage_);
        std::exchange(task.functor_, nullptr)->~functor_base_t();
    } else {
        functor_ = std::exchange(task.functor_, nullptr);
    }

    pending_.store(functor_ != nullptr, std::memory_order_release);
    task.pending_.store(false, std::memory_order_release);
}

auto Task::operator=(Task&& rhs) noexcept -> Task&
{
    assert(rhs.functor_);

    if (rhs.storage_ == reinterpret_cast<std::byte*>(rhs.functor_)) {
        functor_ = rhs.functor_->move_to(storage_);
        std::exchange(rhs.functor_, nullptr)->~functor_base_t();
    } else {
        functor_ = std::exchange(rhs.functor_, nullptr);
    }

    pending_.store(functor_ != nullptr, std::memory_order_release);
    rhs.pending_.store(false, std::memory_order_release);

    return *this;
}

void Task::operator()()
{
    assert(functor_);
    functor_->invoke();
}

void Task::_reset()
{
    if (storage_ == reinterpret_cast<std::byte*>(functor_)) {
        functor_->~functor_base_t();
    } else if (functor_ != nullptr) {
        delete functor_;
    }
    functor_ = nullptr;
}

Task::~Task()
{
    _reset();
}
}