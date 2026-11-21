#pragma once

#include <cassert>
#include <concepts>
#include <iterator>

namespace detail
{

template <std::integral T, std::input_iterator It, std::sentinel_for<It> S>
constexpr auto constexpr_stoi(It begin, S end)
{
    bool neg{false};
    if (begin != end && (*begin == '-' || *begin == '+'))
    {
        neg = (*begin == '-');
        ++begin;
    }

    if constexpr (std::unsigned_integral<T>)
    {
        assert(!neg);
    }

    T sum{0};
    for (; begin != end; ++begin)
    {
        sum = sum * 10 + (*begin - '0');
    }
    return neg ? -sum : sum;
}

template <typename P>
concept has_action = requires { P::action; };

}  // namespace detail
