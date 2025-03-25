//
// Created by anton on 3/23/25.
//

#ifndef TASKWEAVER_UTILITY_H
#define TASKWEAVER_UTILITY_H

#include "common.h"
#include "taskManager.h"
#include <metrix/containers.h>
#include <metrix/type_traits.h>
#include <tuple>

namespace taskweaver {
struct void_future_result_t
{};

template<typename R>
struct when_any_result_t
{
    R result;
    size_t index;
};

namespace internal {
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
    using type_t = void_future_result_t;
};

template<typename R>
auto get_one_future_result(std::future<R>&& f) -> std::conditional_t<std::is_same_v<void, R>, void_future_result_t, R>
{
    if constexpr (std::is_same_v<R, void>) {
        f.get();
        return void_future_result_t{};
    } else {
        return f.get();
    }
}

template<typename R>
auto try_get_one_future_result(std::future<R>&& f)
  -> std::optional<std::conditional_t<std::is_same_v<void, R>, void_future_result_t, R>>
{
    auto r = std::optional<std::conditional_t<std::is_same_v<void, R>, void_future_result_t, R>>{};

    if (f.wait_for(std::chrono::microseconds{ 10 }) == std::future_status::ready) {
        r = internal::get_one_future_result(std::move(f));
    }

    return r;
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
    auto futures = std::make_tuple(std::move(f)...);

    Executor::ThreadExecutor().SubmitTask([p = std::move(promise), fs = std::move(futures)]() mutable -> void {
        []<size_t... I>(std::index_sequence<I...>, auto&& futures, auto&& promise)->void
        {
            promise.set_value(std::make_tuple(internal::get_one_future_result(std::move(std::get<I>(futures)))...));
        }
        (std::make_index_sequence<std::tuple_size_v<decltype(fs)>>{}, std::move(fs), std::move(p));
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

    Executor::ThreadExecutor().SubmitTask([&container, v = std::move(v), p = std::move(promise)]() mutable -> void {
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

    Executor::ThreadExecutor().SubmitTask([first, last, v = std::move(v), p = std::move(promise)]() mutable -> void {
        for (auto it = first; it != last; it++) {
            v.emplace_back(internal::get_one_future_result(*it));
        }
        p.set_value(v);
    });

    return future;
}

namespace internal {
template<size_t I, typename ResultTuple, typename F, typename... Futures>
auto set_when_any_result(when_any_result_t<ResultTuple>& result, F&& future, Futures&&... futures) -> bool
{
    auto&& v = internal::try_get_one_future_result(std::move(future));
    if (v.has_value()) {
        std::get<I>(result.result) = std::move(v.value());
        result.index = I;

        return true;
    }

    if constexpr (sizeof...(futures) > 0) {
        set_when_any_result<I + 1>(result, std::move(futures)...);
    }

    return false;
}
} // internal

template<FutureConcept... F>
auto when_any(F&&... f) -> std::future<when_any_result_t<std::tuple<future_type_t<F>...>>>
{
    auto result = when_any_result_t<std::tuple<future_type_t<F>...>>{};
    auto promise = std::promise<when_any_result_t<std::tuple<future_type_t<F>...>>>{};
    auto future = promise.get_future();
    auto futures = std::make_tuple(std::move(f)...);

    Executor::ThreadExecutor().SubmitTask(
      [r = std::move(result), p = std::move(promise), fs = std::move(futures)]() mutable -> void {
          []<size_t... I>(std::index_sequence<I...>, auto& result, auto&& futures, auto&& promise)->void
          {
              while (!internal::set_when_any_result<0>(result, std::move(std::get<I>(futures))...)) {
              }
              promise.set_value(std::move(result));
          }
          (std::make_index_sequence<std::tuple_size_v<decltype(fs)>>{}, r, std::move(fs), std::move(p));
      });

    return future;
}

template<FutureContainerConcept C>
auto when_any(C const& container) -> std::future<when_any_result_t<future_type_t<typename C::value_type>>>
{
    auto result = when_any_result_t<future_type_t<typename C::value_type>>{};
    auto promise = std::promise<when_any_result_t<future_type_t<typename C::value_type>>>{};
    auto future = promise.get_future();

    Executor::ThreadExecutor().SubmitTask(
      [&container, r = std::move(result), p = std::move(promise)]() mutable -> void {
          auto any_result = false;
          while (!any_result) {
              auto i = size_t{ 0 };
              for (auto&& f : container) {
                  if (auto opt = internal::try_get_one_future_result(std::move(f)); opt.has_value()) {
                      r.result = opt.value();
                      r.index = i;
                      any_result = true;
                      break;
                  }
                  i++;
              }
          }
          p.set_value(std::move(r));
      });

    return future;
}

template<FutureInteratorConcept InputIt>
auto when_any(InputIt first, InputIt last)
  -> std::future<when_any_result_t<future_type_t<typename std::iterator_traits<InputIt>::value_type>>>
{
    auto result = when_any_result_t<future_type_t<typename std::iterator_traits<InputIt>::value_type>>{};
    auto promise = std::promise<when_any_result_t<future_type_t<typename std::iterator_traits<InputIt>::value_type>>>{};
    auto future = promise.get_future();

    Executor::ThreadExecutor().SubmitTask(
      [first, last, r = std::move(result), p = std::move(promise)]() mutable -> void {
          auto any_result = false;
          while (!any_result) {
              auto i = size_t{ 0 };
              for (auto it = first; it != last; it++) {
                  if (auto opt = internal::try_get_one_future_result(std::move(*it)); opt.has_value()) {
                      r.result = opt.value();
                      r.index = i;
                      any_result = true;
                      break;
                  }
                  i++;
              }
          }
          p.set_value(std::move(r));
      });

    return future;
}
}

#endif // TASKWEAVER_UTILITY_H
