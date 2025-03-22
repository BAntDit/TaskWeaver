//
// Created by anton on 3/16/25.
//

#ifndef TASKWEAVER_COMMON_H
#define TASKWEAVER_COMMON_H

#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <future>
#include <memory>
#include <new>
#include <thread>
#include <type_traits>

#if defined(ALLOW_THREAD_EXCEPTIONS_PROPAGATION)
#include <exception>
#include <mutex>
#endif // ALLOW_THREAD_EXCEPTIONS_PROPAGATION

#ifdef __cpp_lib_hardware_interference_size
using std::hardware_constructive_interference_size;
using std::hardware_destructive_interference_size;
#else
// https://en.cppreference.com/w/cpp/thread/hardware_destructive_interference_size
// 64 bytes on x86-64 | L1_CACHE_BYTES | L1_CACHE_SHIFT | __cacheline_aligned | ...
inline constexpr size_t hardware_constructive_interference_size = 64;
inline constexpr size_t hardware_destructive_interference_size = 64;
#endif

#ifdef __cpp_lib_smart_ptr_for_overwrite
using std::make_unique_for_overwrite;
#else
template<typename T>
auto make_unique_for_overwrite(size_t n) -> std::enable_if_t<is_unbounded_array<T>::value, std::unique_ptr<T>>
{
    return std::unique_ptr<T>(new std::remove_extent_t<T>[n]);
}
#endif

#endif // TASKWEAVER_COMMON_H
