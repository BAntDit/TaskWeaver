//
// Created by anton on 3/22/25.
//

#include "taskManager.h"

namespace taskweaver {
TaskManager::TaskManager(size_t taskQueueSize /* = 256*/,
                         size_t threadPoolSize /* = std::max(std::thread::hardware_concurrency(), 1u)*/)
  : threadPool_{}
  , executorCount_{ threadPoolSize + 1 } // plus main thread
  , executors_{ make_unique_for_overwrite<Executor[]>(executorCount_) }
  , alive_{ true }
#if defined(ALLOW_THREAD_EXCEPTIONS_PROPAGATION)
  , exPropagationMutex_{}
  , exceptions_{}
#endif
{
    for (auto i = size_t{ 0 }; i < executorCount_; i++) {
        new (&executors_[i]) Executor{ *this, taskQueueSize };
    }

    threadPool_.reserve(threadPoolSize);

    executors_[0]._SetAsMainThreadExecutor();
}

TaskManager::~TaskManager()
{
    Stop();

    executors_[0]._ResetMainThreadExecutor();
}

void TaskManager::Start()
{
    assert(Executor::IsInMainThread());

    for (auto i = size_t{ 0 }; i < executorCount_; i++) {
        threadPool_.emplace_back([](Executor& executor) -> void { executor(); }, std::ref(executors_[i + 1]));
    }
}

void TaskManager::Stop()
{
    alive_.store(false);

    for (auto&& thread : threadPool_) {
        if (thread.joinable())
            thread.join();
    }
}

#if defined(ALLOW_THREAD_EXCEPTIONS_PROPAGATION)
void TaskManager::PropagateException(std::exception_ptr const& exception)
{
    std::lock_guard<std::mutex> lock{ exPropagationMutex_ };
    exceptions_.push_back(exception);
}

auto TaskManager::GetLastException() -> std::exception_ptr
{
    std::lock_guard<std::mutex> lock{ exPropagationMutex_ };

    auto ex = std::exception_ptr{};

    if (!exceptions_.empty()) {
        ex = exceptions_.back();
        exceptions_.pop_back();
    }

    return ex;
}
#endif
}
