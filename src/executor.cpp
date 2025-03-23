//
// Created by anton on 3/20/25.
//

#include "executor.h"
#include "taskManager.h"
#include <random>

#if defined(ALLOW_THREAD_EXCEPTIONS_PROPAGATION)
#define BEGIN_EXCEPTION_PROPAGATION() try {

#define END_EXCEPTION_PROPAGATION()                                                                                    \
    }                                                                                                                  \
    catch (...)                                                                                                        \
    {                                                                                                                  \
        TaskManager().PropagateException(std::current_exception());                                                    \
    }
#else
#define BEGIN_EXCEPTION_PROPAGATION()
#define END_EXCEPTION_PROPAGATION()
#endif

namespace taskweaver {
namespace {
thread_local Executor* _mainThreadExecutor = nullptr;
thread_local Executor* _threadExecutor = nullptr;

auto randomExecutorIndex(size_t executorCount) -> size_t
{
    assert(executorCount > 0);

    static thread_local auto rd = std::random_device();
    static thread_local auto generator = std::default_random_engine{ rd() };

    std::uniform_int_distribution<int32_t> distribution(0, static_cast<int>(executorCount) - 1);

    return static_cast<size_t>(distribution(generator));
}
}

/*static*/ auto Executor::IsInMainThread() -> bool
{
    return _mainThreadExecutor != nullptr;
}

/*static*/ auto Executor::ThreadExecutor() -> Executor&
{
    assert(_threadExecutor != nullptr);
    return *_threadExecutor;
}

Executor::Executor(taskweaver::TaskManager& taskManager, size_t taskDequeSize)
  : threadId_{}
  , taskManager_{ &taskManager }
  , taskPool_{ taskDequeSize * 2 } // max tasks per executor
  , taskQueue_{ nullptr }
{
    taskQueue_ = std::make_unique<TaskStealingDeque<Task*>>(taskDequeSize);
}

auto Executor::CanSubmit() const -> bool
{
    // task submition is posible from the owner thread only
    return (_threadExecutor != nullptr) && threadId_ == std::this_thread::get_id();
}

void Executor::RunOne()
{
    if (auto task = PendingTask()) {
        task.value()();
    } else {
        std::this_thread::yield();
    }
}

void Executor::Run()
{
    if (IsInMainThread())
        return;

    _SetThreadExecutorPtr();

    BEGIN_EXCEPTION_PROPAGATION();

    while (TaskManager().KeepAlive()) {
        RunOne();
    }

    END_EXCEPTION_PROPAGATION();

    _ResetThreadExecutorPtr();
}

void Executor::_SetThreadExecutorPtr()
{
    assert(_threadExecutor == nullptr);
    _threadExecutor = this;

    threadId_ = std::this_thread::get_id();
}

void Executor::_ResetThreadExecutorPtr()
{
    _threadExecutor = nullptr;

    threadId_ = std::thread::id{};
}

void Executor::_SetAsMainThreadExecutor()
{
    assert(_mainThreadExecutor == nullptr);
    _mainThreadExecutor = this;
    _threadExecutor = this;

    threadId_ = std::this_thread::get_id();
}

void Executor::_ResetMainThreadExecutor()
{
    _mainThreadExecutor = nullptr;
    _threadExecutor = nullptr;

    threadId_ = std::thread::id{};
}

void Executor::operator()()
{
    Run();
}

auto Executor::PendingTask() -> std::optional<Task>
{
    auto task = std::optional<Task>{ std::nullopt };

    if (auto ownTask = Queue().TryPop()) {
        task = std::move(*ownTask.value());
    } else {
        auto executorCount = TaskManager().ExecutorCount();
        auto executorIndex = randomExecutorIndex(executorCount);
        auto& executors = TaskManager().Executors();
        auto& executor = executors[executorIndex];

        if (auto stolenTask = executor.Queue().TrySteal()) {
            task = std::move(*stolenTask.value());
        }
    }

    return task;
}
}
