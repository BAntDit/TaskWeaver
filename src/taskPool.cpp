//
// Created by anton on 3/20/25.
//

#include "taskPool.h"

namespace taskweaver {
TaskPool::TaskPool(size_t size)
  : size_{ size }
  , tasks_{ make_unique_for_overwrite<Task[]>(size) }
{
}

auto TaskPool::WriteableTask() -> Task*
{
    auto* taskPtr = std::add_pointer_t<Task>{ nullptr };

    for (auto i = size_t{ 0 }; i < size_; i++) {
        auto& task = tasks_[i];

        // it is safe, because each executor has own pool
        // so, false negative condition because of races is not possible
        if (!task.pending()) {
            taskPtr = &task;
            break;
        }
    }

    return taskPtr;
}
}
