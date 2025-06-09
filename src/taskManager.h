//
// Created by anton on 3/20/25.
//

#ifndef TASKWEAVER_TASKMANAGER_H
#define TASKWEAVER_TASKMANAGER_H

#include "common.h"
#include "executor.h"

namespace taskweaver {
class TaskManager
{
    friend class Executor;

public:
    TaskManager(size_t taskQueueSize = 256, size_t threadPoolSize = std::max(std::thread::hardware_concurrency(), 1u));

    TaskManager(TaskManager const&) = delete;

    TaskManager(TaskManager&&) = delete;

    ~TaskManager();

    auto operator=(TaskManager const&) -> TaskManager& = delete;

    auto operator=(TaskManager&&) -> TaskManager& = delete;

    [[nodiscard]] auto KeepAlive() const -> bool { return alive_.load(std::memory_order_relaxed); }

    [[nodiscard]] auto ExecutorCount() const -> uint32_t { return executorCount_; }

    void Start();

    void Stop();

    template<typename F>
    static auto SubmitTask(F&& f) -> std::enable_if_t<std::is_invocable_v<F>, std::future<std::invoke_result_t<F>>>;

#if defined(ALLOW_THREAD_EXCEPTIONS_PROPAGATION)
    auto GetLastException() -> std::exception_ptr;
#endif

private:
    [[nodiscard]] auto Executors() const -> std::unique_ptr<Executor[]> const& { return executors_; }

    [[nodiscard]] auto Executors() -> std::unique_ptr<Executor[]>& { return executors_; }

#if defined(ALLOW_THREAD_EXCEPTIONS_PROPAGATION)
    void PropagateException(std::exception_ptr const& exception);
#endif

private:
    std::vector<std::thread> threadPool_;

    size_t executorCount_;
    std::unique_ptr<Executor[]> executors_;

    std::atomic<bool> alive_;

#if defined(ALLOW_THREAD_EXCEPTIONS_PROPAGATION)
    std::mutex exPropagationMutex_;
    std::vector<std::exception_ptr> exceptions_;
#endif
};

template<typename F>
/*static*/ auto TaskManager::SubmitTask(F&& f)
  -> std::enable_if_t<std::is_invocable_v<F>, std::future<std::invoke_result_t<F>>>
{
    return Executor::ThreadExecutor().SubmitTask(std::forward<F>(f));
}
}

#endif // TASKWEAVER_TASKMANAGER_H
