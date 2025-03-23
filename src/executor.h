//
// Created by anton on 3/20/25.
//

#ifndef TASKWEAVER_EXECUTOR_H
#define TASKWEAVER_EXECUTOR_H

#include "common.h"
#include "taskPool.h"
#include "taskStealingDeque.h"

namespace taskweaver {
class TaskManager;

class Executor
{
    friend class TaskManager;

public:
    Executor() = default;

    Executor(taskweaver::TaskManager& taskManager, size_t taskDequeSize);

    Executor(Executor const&) = delete;

    Executor(Executor&&) = default;

    ~Executor() = default;

    auto operator=(Executor const&) -> Executor& = delete;

    auto operator=(Executor&&) -> Executor& = default;

    void operator()();

    void Run();

    void RunOne();

    template<typename F>
    auto SubmitTask(F&& f) -> std::enable_if_t<std::is_invocable_v<F>, std::future<std::result_of_t<F()>>>;

    [[nodiscard]] auto OwnerThreadId() const -> std::thread::id { return threadId_; }

    [[nodiscard]] auto CanSubmit() const -> bool;

    [[nodiscard]] auto TaskManager() const -> taskweaver::TaskManager const& { return *taskManager_; }

    [[nodiscard]] auto TaskManager() -> taskweaver::TaskManager& { return *taskManager_; }

    static auto IsInMainThread() -> bool;

    static auto ThreadExecutor() -> Executor&;

private:
    [[nodiscard]] auto Pool() const -> TaskPool const& { return taskPool_; }
    [[nodiscard]] auto Pool() -> TaskPool& { return taskPool_; }

    [[nodiscard]] auto Queue() const -> TaskStealingDeque<Task*> const& { return *taskQueue_; }
    [[nodiscard]] auto Queue() -> TaskStealingDeque<Task*>& { return *taskQueue_; }

    auto PendingTask() -> std::optional<Task>;

    void _SetThreadExecutorPtr();

    void _ResetThreadExecutorPtr();

    void _SetAsMainThreadExecutor();

    void _ResetMainThreadExecutor();

private:
    std::thread::id threadId_;
    taskweaver::TaskManager* taskManager_;
    TaskPool taskPool_;
    std::unique_ptr<TaskStealingDeque<Task*>> taskQueue_;
};

template<typename F>
auto Executor::SubmitTask(F&& f) -> std::enable_if_t<std::is_invocable_v<F>, std::future<std::result_of_t<F()>>>
{
    assert(CanSubmit());

    using result_type_t = std::result_of_t<F()>;

    auto* task = std::add_pointer_t<Task>{ nullptr };

    // cycle waits a task from pool if there is no one
    // it must happen hardly ever
    while ((task = Pool().WriteableTask(), task == nullptr))
        std::this_thread::yield();

    auto&& packedTask = std::packaged_task<result_type_t()>{ std::forward<F>(f) };
    auto future = packedTask.get_future();

    *task = Task{ std::move(packedTask) };

    // cycle waits space in deque if there is no one
    // it must happen hardly ever as well
    while (!Queue().TryEmplace(task))
        std::this_thread::yield();

    return future;
}
}

#endif // TASKWEAVER_EXECUTOR_H
