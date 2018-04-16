#pragma once

#include <tuple>
#include "TypeList.h"

namespace ecs {

template <std::size_t...>
struct Sequence {};

template <std::size_t N, std::size_t... Seq>
struct GenerateSequence : GenerateSequence<N - 1, N - 1, Seq...> {};

template <std::size_t... Seq>
struct GenerateSequence<0, Seq...> {
    using Type = Sequence<Seq...>;
};

template <typename F>
struct FunctionTraits;

template <typename R, typename... A>
struct FunctionTraits<R(*)(A...)> : public FunctionTraits<R(A...)> {};

template <typename R, typename... A>
struct FunctionTraits<R(A...)> {
    static constexpr std::size_t ArgCount = sizeof...(A);
    using Args = TypeList<A...>;
    using ReturnType = R;

    template <std::size_t I>
    using Arg = typename Args::template Get<I>;
};

template <typename C, typename R, typename... A>
struct FunctionTraits<R(C::*)(A...)> : public FunctionTraits<R(A...)> {};

template <typename C, typename R, typename... A>
struct FunctionTraits<R(C::*)(A...) const> : public FunctionTraits<R(A...)> {};

template <typename C, typename R>
struct FunctionTraits<R(C::*)> : public FunctionTraits<R()> {};

template <typename F>
struct FunctionTraits {
private:
    using FTraits = FunctionTraits<decltype(&F::operator())>;
public:
    static constexpr std::size_t ArgCount = FTraits::ArgCount;
    using Args = typename FTraits::Args;
    using ReturnType = typename FTraits::ReturnType;

    template <std::size_t I>
    using Arg = typename FTraits::template Arg<I>;
};

template <typename F>
struct FunctionTraits<F&> : public FunctionTraits<F> {};

template <typename F>
struct FunctionTraits<F&&> : public FunctionTraits<F> {};

}