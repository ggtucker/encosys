#pragma once

#include <tuple>

namespace ecs {
namespace tmp {

// Check if all types in variadic templates of the given type are equal to the expected value

template <typename T, T...> struct _Value_Pack;
template <typename T, T Expected, T... Values>
using _All_Equal_To = std::is_same<_Value_Pack<T, Values..., Expected>, _Value_Pack<T, Expected, Values...>>;

template <typename T, T Expected, T... Values>
static constexpr bool AllEqualTo = _All_Equal_To<T, Expected, Values...>::value;

// Repeat

template <typename, typename>
struct _Repeat_Append;

template <typename T, typename... Ts>
struct _Repeat_Append<T, std::tuple<Ts...>> {
    using Type = std::tuple<Ts..., T>;
};

template <std::size_t N, typename T>
struct _Repeat {
    using Type = typename _Repeat_Append<T, typename _Repeat<N - 1, T>::Type>::Type;
};

template <typename T>
struct _Repeat<0, T> {
    using Type = std::tuple<>;
};

template <std::size_t N, typename T>
using Repeat = typename _Repeat<N, T>::Type;

} // namespace tmp
} // namespace ecs
