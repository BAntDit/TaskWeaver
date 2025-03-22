//
// Created by anton on 3/20/25.
//

#ifndef TASKWEAVER_TASKPOOL_H
#define TASKWEAVER_TASKPOOL_H

#include "task.h"

namespace taskweaver {
class TaskPool
{
public:
    TaskPool() = default;

    explicit TaskPool(size_t size);

    TaskPool(TaskPool const&) = delete;

    TaskPool(TaskPool&&) = default;

    ~TaskPool() = default;

    auto operator=(TaskPool const&) -> TaskPool& = delete;

    auto operator=(TaskPool&&) -> TaskPool& = default;

    auto WriteableTask() -> Task*;

private:
    size_t size_;
    std::unique_ptr<Task[]> tasks_;
};
}

#endif // TASKWEAVER_TASKPOOL_H
