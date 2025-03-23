//
// Created by anton on 3/23/25.
//

#ifndef TASKWEAVER_UTILITY_H
#define TASKWEAVER_UTILITY_H

#include "common.h"
#include "taskManager.h"
#include <metrix/containers.h>
#include <metrix/type_traits.h>

namespace taskweaver {
namespace internal {
struct void_result_t
{};

template<typename F>
struct get_future_type;

template<typename T>
struct get_future_type<std::future<T>>
{
    using type_t = T;
};

template<>
struct get_future_type<std::future<void>>
{
    using type_t = void_result_t;
};

template<typename R>
auto get_one_future_result(std::future<R>&& f)
  -> std::conditional_t<std::is_same_v<void, R>, internal::void_result_t, R>
{
    if constexpr (std::is_same_v<R, void>) {
        f.get();
        return internal::void_result_t{};
    } else {
        return f.get();
    }
}
} // internal

template<typename F>
concept FutureConcept = metrix::is_specialization_of_v<F, std::future>;

template<typename C>
concept FutureContainerConcept =
  metrix::is_iterable_v<C> && metrix::is_specialization_of_v<typename C::value_type, std::future>;

template<typename I>
concept FutureInteratorConcept =
  std::input_iterator<I> && metrix::is_specialization_of_v<typename std::iterator_traits<I>::value_type, std::future>;

template<FutureConcept F>
using future_type_t = typename internal::get_future_type<F>::type_t;

template<FutureConcept... F>
auto when_all(F&&... f) -> std::future<std::tuple<future_type_t<F>...>>
{
    auto promise = std::promise<std::tuple<future_type_t<F>...>>{};
    auto future = promise.get_future();
    auto futures = std::forward_as_tuple(std::forward<F>(f)...);

    Executor::ThreadExecutor().SubmitTask([p = std::move(promise), fs = std::move(futures)]() -> void {
        []<size_t... I>(std::index_sequence<I...>, auto& futures, auto& promise)->void
        {
            promise.set_value(std::make_tuple(internal::get_one_future_result(std::get<I>(futures))...));
        }
        (std::make_index_sequence<std::size(futures)>{}, std::move(fs), std::move(p));
    });

    return future;
}

template<FutureContainerConcept C>
auto when_all(C const& container) -> std::future<std::vector<future_type_t<typename C::value_type>>>
{
    auto v = std::vector<future_type_t<typename C::value_type>>{};
    v.reserve(std::size(container));

    auto promise = std::promise<decltype(v)>{};
    auto future = promise.get_future();

    Executor::ThreadExecutor().SubmitTask([&container, v = std::move(v), p = std::move(promise)]() -> void {
        for (auto&& f : container) {
            v.emplace_back(internal::get_one_future_result(f));
        }
        p.set_value(v);
    });

    return future;
}

template<FutureInteratorConcept InputIt>
auto when_all(InputIt first, InputIt last)
  -> std::future<std::vector<future_type_t<typename std::iterator_traits<InputIt>::value_type>>>
{
    auto v = std::vector<future_type_t<typename std::iterator_traits<InputIt>::value_type>>{};
    v.reserve(std::distance(first, last));

    auto promise = std::promise<decltype(v)>{};
    auto future = promise.get_future();

    Executor::ThreadExecutor().SubmitTask([first, last, v = std::move(v), p = std::move(promise)]() -> void {
        for (auto it = first; it != last; it++) {
            v.emplace_back(internal::get_one_future_result(*it));
        }
        p.set_value(v);
    });

    return future;
}
}

#endif // TASKWEAVER_UTILITY_H
