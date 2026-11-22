#pragma once

#include <meta>
#include <optional>
#include <utility>

#include "details.h"
#include "values.h"

namespace rp
{
namespace parser
{

template <typename P>
concept Production = requires {
    typename P::match;
} && (!detail::has_action<P> || std::meta::is_invocable_type(^^decltype(P::action)&, P::match::value_type()));

template <char C>
struct literal : emits_nothing
{
    template <typename It, typename Next, typename... Args>
    static constexpr auto parse(It& cur, It end, Next&& next, Args&&... args)
        -> decltype(next(cur, end, std::forward<Args>(args)...))
    {
        if (cur == end || *cur != C)
        {
            return {};  // a std::optional<T> of {} is 'empty'
        }
        ++cur;
        return next(cur, end, std::forward<Args>(args)...);
    }
};

struct period : emits_nothing
{
    template <typename It, typename Next, typename... Args>
    static constexpr auto parse(It& cur, It end, Next&& next, Args&&... args)
        -> decltype(next(cur, end, std::forward<Args>(args)...))
    {
        if (cur == end)
        {
            return {};
        }
        ++cur;
        return next(cur, end, std::forward<Args>(args)...);
    }
};

template <Production P, typename It>
static constexpr auto parse_with_action(It begin, It end)
{
    auto final_parser = [](It& cur, It end, auto&&... vals)
    { return std::optional{P::action(std::forward<decltype(vals)>(vals)...)}; };

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
