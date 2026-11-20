#pragma once

#include <utility>
#include <cassert>
#include <string_view>

namespace
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

    if constexpr (std::unsigned_integral<T>) {
        assert(!neg);
    }

    T sum{0};
    for (; begin != end; ++begin)
    {
        sum = sum * 10 + (*begin - '0');
    }
    return neg ? -sum : sum;
}

} // namespace

namespace rp
{
namespace parser
{

template <char C>
struct literal
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

struct period
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
struct star
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
struct plus
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
    template <typename It, typename Next, typename... Args>
    static constexpr auto parse(It& cur, It end, Next&& next, Args&&... args)
    {
        return next(cur, end, std::forward<Args>(args)...);
    }
};

template <typename First, typename... Rest>
struct branch
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
struct branch<First>
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
    template <typename It, typename Next, typename... Args>
    static constexpr auto parse(It& cur, It end, Next&& next, Args&&... args)
    {
        if constexpr (std::integral<T>)
        {
            return next(cur, end, std::forward<Args>(args)..., constexpr_stoi<T>(cur, end));
        }
        else
        {
            return next(cur, end, std::forward<Args>(args)..., std::string_view(cur, end));
        }
    }
};

template <typename P, typename It>
static constexpr auto parse_with_action(It begin, It end)
{
    constexpr auto final_parser = [](It& cur, It end, auto&&... vals)
    {
        return P::action(std::forward<decltype(vals)>(vals)...);
    };
    return P::match::parse(begin, end, final_parser);
}

template <typename P, typename It>
static constexpr auto match(It begin, It end)
{
    auto final_parser = [](It& cur, It end, auto&&... args) { return cur == end; };
    return P::match::parse(begin, end, final_parser);
}

}  // namespace parser
}  // namespace rp
