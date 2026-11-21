#pragma once

#include <concepts>
#include <string_view>
#include <utility>

#include "details.h"

namespace rp
{
namespace parser
{

template <typename P>
concept Production = requires {
    typename P::match;
    typename P::match::value_type;
} && (!detail::has_action<P> || std::invocable<decltype(P::action)&, typename P::match::value_type>); // & necessary?

struct matcher
{
    using value_type = void;
};

template <char C>
struct literal : matcher
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

struct period : matcher
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
struct star : matcher
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
struct plus : matcher
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
struct seq<First, Rest...> : matcher // TEMPORARY workaround
{
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
struct seq<> : matcher // TEMPORARY workaround
{
    template <typename It, typename Next, typename... Args>
    static constexpr auto parse(It& cur, It end, Next&& next, Args&&... args)
    {
        return next(cur, end, std::forward<Args>(args)...);
    }
};

template <typename First, typename... Rest>
struct branch : matcher
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
struct branch<First> : matcher
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
    using value_type = T;

    template <typename It, typename Next, typename... Args>
    static constexpr auto parse(It& cur, It end, Next&& next, Args&&... args)
    {
        if constexpr (std::integral<T>)
        {
            return next(cur, end, std::forward<Args>(args)..., detail::constexpr_stoi<T>(cur, end));
        }
        else
        {
            return next(cur, end, std::forward<Args>(args)..., std::string_view(cur, end));
        }
    }
};

template <Production P, typename It>
static constexpr auto parse_with_action(It begin, It end)
{
    constexpr auto final_parser = [](It& cur, It end, auto&&... vals)
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
