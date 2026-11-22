#pragma once

#include <concepts>
#include <string_view>
#include <utility>
#include <vector>
#include <meta>

#include "details.h"

namespace rp
{
namespace parser
{

template <typename P>
concept Production = requires {
    typename P::match;
} && (!detail::has_action<P> || std::meta::is_invocable_type(^^decltype(P::action)&, P::match::value_type()));

struct emits_nothing
{
    static consteval std::vector<std::meta::info> value_type() { return {}; };
};

template <char C>
struct literal : emits_nothing
{
    template <typename It, typename Next, typename... Args>
    static constexpr auto parse(It& cur, It end, Next&& next, Args&&... args)
    {
        if (cur == end || *cur != C)
        {
            return false;
        }
        ++cur;
        return next(cur, end, std::forward<Args>(args)...);
    }
};

struct period : emits_nothing
{
    template <typename It, typename Next, typename... Args>
    static constexpr auto parse(It& cur, It end, Next&& next, Args&&... args)
    {
        if (cur == end)
        {
            return false;
        }
        ++cur;
        return next(cur, end, std::forward<Args>(args)...);
    }
};

template <typename Production>
struct star : emits_nothing
{
    template <typename Iterator>
    static constexpr auto match(Iterator& cur, Iterator end)
    {
        if (cur == end || !Production::match(cur, end))
        {
            return true;
        }
        return match(cur, end);
    }
};

template <typename Production>
struct plus : emits_nothing
{
    template <typename Iterator>
    static constexpr auto match(Iterator& cur, Iterator end)
    {
        return Production::match(cur, end) && star<Production>::match(cur, end);
    }
};

template <typename... Ps>
struct seq;

template <typename First, typename... Rest>
struct seq<First, Rest...>
{
    static consteval std::vector<std::meta::info> value_type()
    {
        std::vector<std::meta::info> v{ First::value_type() };
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

template <typename First, typename... Rest>
struct branch : emits_nothing // TEMPORARY
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

template <typename First>
struct branch<First> : emits_nothing // TEMPORARY
{
    template <typename Iterator>
    static constexpr auto match(Iterator& cur, Iterator end)
    {
        return First::match(cur, end);
    }
};

template <typename P, typename T = std::string_view>
struct emit
{
    static consteval std::vector<std::meta::info> value_type() { return {^^T}; };

    template <typename It, typename Next, typename... Args>
    static constexpr auto parse(It& cur, It end, Next&& next, Args&&... args)
    {
        // P::parse() will bump cur to where the match ended, so matched string is from start -> where cur is at end
        const It start = cur;
        auto continuation = [&start, &next, &args...](It& cur, It end, auto&&... vals)
        {
            const std::string_view text(start, cur);
            if constexpr (std::integral<T>)
            {
                return next(cur, end, std::forward<Args>(args)..., detail::constexpr_stoi<T>(text.begin(), text.end()));
            }
            else
            {
                return next(cur, end, std::forward<Args>(args)..., text);
            }
        };

        return P::parse(cur, end, continuation);
    }
};

template <Production P, typename It>
static constexpr auto parse_with_action(It begin, It end)
{
    auto final_parser = [](It& cur, It end, auto&&... vals)
    { return P::action(std::forward<decltype(vals)>(vals)...); };
    return P::match::parse(begin, end, final_parser);
}

template <Production P, typename It>
static constexpr auto match(It begin, It end)
{
    auto final_parser = [](It& cur, It end, auto&&... args) { return cur == end; };
    return P::match::parse(begin, end, final_parser);
}

}  // namespace parser
}  // namespace rp
