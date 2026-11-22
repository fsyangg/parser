#pragma once

#include <concepts>
#include <meta>
#include <string_view>
#include <utility>
#include <vector>

#include "details.h"

namespace rp
{
namespace parser
{

struct emits_nothing
{
    static consteval std::vector<std::meta::info> value_type() { return {}; };
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

}  // namespace parser
}  // namespace rp
