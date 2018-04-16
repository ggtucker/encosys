#pragma once

#include <tuple>
#include <functional>

#define TYPE_OF(t) typename decltype(t)::Type

namespace ecs {

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

// Type List

template <typename... TList>
struct TypeList {

    using ListTuple = std::tuple<TList...>;

    // Size of list
    static constexpr std::size_t Size = std::tuple_size<ListTuple>::value;

    // Get type at index
    template <std::size_t N>
    using Get = std::tuple_element_t<N, ListTuple>;

    // Append to list
    template <typename T>
    struct Append {
        using Type = TypeList<TList..., T>;
    };

    // Prepend to list
    template <typename T>
    struct Prepend {
        using Type = TypeList<T, TList...>;
    };

    // Remove first type from type list
    template <typename T>
    struct _RemoveFirst;

    template <>
    struct _RemoveFirst<TypeList<>> {
        using Type = TypeList<>;
    };

    template <typename T, typename... Types>
    struct _RemoveFirst<TypeList<T, Types...>> {
        using Type = TypeList<Types...>;
    };

    using RemoveFirst = typename _RemoveFirst<TypeList<TList...>>::Type;

    // Filter list
    template <template <typename> class, typename, typename TResult>
    struct _Filter {
        using Type = TResult;
    };

    template <template <typename> class TFilter, typename T, typename... T1s, typename TResult>
    struct _Filter<TFilter, TypeList<T, T1s...>, TResult> {
        using ExcludedList = typename _Filter<TFilter, TypeList<T1s...>, TResult>::Type;
        using AddedList = typename _Filter<TFilter, TypeList<T1s...>, typename TResult::template Append<T>::Type>::Type;
        using Type = std::conditional_t<!TFilter<T>::value, ExcludedList, AddedList>;
    };

    template <template <typename> class TFilter>
    using Filter = typename _Filter<TFilter, TypeList<TList...>, TypeList<>>::Type;

    // Wrap types in type list
    template <template <typename> class, typename, typename TResult>
    struct _WrapTypes {
        using Type = TResult;
    };

    template <template <typename> class TWrapper, typename T, typename... T1s, typename TResult>
    struct _WrapTypes<TWrapper, TypeList<T, T1s...>, typename TResult> {
        using Type = typename _WrapTypes<TWrapper, TypeList<T1s...>, typename TResult::template Append<TWrapper<T>>::Type>::Type;
    };

    template <template <typename> class TWrapper>
    using WrapTypes = typename _WrapTypes<TWrapper, TypeList<TList...>, TypeList<>>::Type;

    // Check if contains type
    template <typename T, typename Tuple>
    struct _Contains;

    template <typename T>
    struct _Contains<T, std::tuple<>> {
        static const bool Value = false;
    };

    template <typename T, typename... Types>
    struct _Contains<T, std::tuple<T, Types...>> {
        static const bool Value = true;
    };

    template <typename T, typename U, typename... Types>
    struct _Contains<T, std::tuple<U, Types...>> {
        static const bool Value = _Contains<T, std::tuple<Types...>>::Value;
    };

    template <typename T>
    static constexpr bool Contains () {
        return _Contains<T, ListTuple>::Value;
    }

    // Get index of type
    template <typename T, typename Tuple>
    struct _IndexOf;

    template <typename T, typename... Types>
    struct _IndexOf<T, std::tuple<T, Types...>> {
        static const std::size_t Value = 0;
    };

    template <typename T, typename U, typename... Types>
    struct _IndexOf<T, std::tuple<U, Types...>> {
        static const std::size_t Value = 1 + _IndexOf<T, std::tuple<Types...>>::Value;
    };

    template <typename T>
    static constexpr std::size_t IndexOf () {
        static_assert(Contains<T>(), "IndexOf() requires type list to contain type");
        return _IndexOf<T, ListTuple>::Value;
    }

    // Rename type list to use different wrapper of TList...
    template <template <typename...> class T>
    using Rename = T<TList...>;

    // Execute lambda on each type
    template <typename T>
    struct TypeWrapper {
        using Type = T;
    };

    template <typename TFunc>
    static constexpr void ForTypes (TFunc&& func) {
        (void)std::initializer_list<int>{(func(TypeWrapper<TList>()), 0)...};
    };
};

}