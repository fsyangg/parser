#pragma once

#include <meta>
#include <optional>
#include <utility>
#include <vector>

#include "values.h"

namespace rp
{
namespace parser
{

// TODO
template <typename Production>
struct star : emits_nothing
{
    template <typename Iterator>
    static constexpr auto parse(Iterator& cur, Iterator end)
    {
        if (cur == end || !Production::match(cur, end))
        {
            return std::nullopt;
        }
        return match(cur, end);
    }
};

// TODO
template <typename Production>
struct plus : emits_nothing
{
    template <typename Iterator>
    static constexpr auto parse(Iterator& cur, Iterator end)
    {
        return Production::parse(cur, end) && star<Production>::parse(cur, end);
    }
};

template <typename... Ps>
struct seq;

template <typename First, typename... Rest>
struct seq<First, Rest...>
{
    static consteval std::vector<std::meta::info> value_type()
    {
        std::vector<std::meta::info> v{First::value_type()};
        (v.append_range(Rest::value_type()), ...);
        return v;
    }

    template <typename It, typename Next, typename... Args>
    static constexpr auto parse(It& cur, It end, Next&& next, Args&&... args)
    {
        // construct the continuation for the first parser, which is a call to the second parser, etc.
        // use auto&&... vals instead of args so first parser can add when it calls continuation
        auto continuation = [&next](It& cur, It end, auto&&... vals)
        { return seq<Rest...>::parse(cur, end, next, std::forward<decltype(vals)>(vals)...); };

        return First::parse(cur, end, continuation, std::forward<Args>(args)...);
    }
};

template <>
struct seq<>
{
    static consteval std::vector<std::meta::info> value_type() { return {}; };

    template <typename It, typename Next, typename... Args>
    static constexpr auto parse(It& cur, It end, Next&& next, Args&&... args)
    {
        return next(cur, end, std::forward<Args>(args)...);
    }
};

// TODO
template <typename First, typename... Rest>
struct branch : emits_nothing  // TEMPORARY
{
    template <typename Iterator>
    static constexpr auto match(Iterator& cur, Iterator end)
    {
        const auto copy = cur;
        if (First::match(cur, end))
        {
            return true;
        }
        cur = copy;
        return (Rest::match(cur, end) || ...);
    }
};

// TODO
template <typename First>
struct branch<First> : emits_nothing  // TEMPORARY
{
    template <typename Iterator>
    static constexpr auto match(Iterator& cur, Iterator end)
    {
        return First::match(cur, end);
    }
};

}  // namespace parser
}  // namespace rp
