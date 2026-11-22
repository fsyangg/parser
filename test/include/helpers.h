#pragma once

#include <algorithm>
#include <initializer_list>
#include <string_view>

#include "parsers.h"

namespace rp
{
namespace tests
{

template <typename Prod, auto Func>
struct parse_func
{
    static constexpr bool operator()(std::initializer_list<std::string_view> strs)
    {
        return Func(strs, [](auto s) { return parser::match<Prod>(s.begin(), s.end()); });
    }
};

template <typename Prod>
constexpr parse_func<Prod, std::ranges::all_of> all_parse{};

template <typename Prod>
constexpr parse_func<Prod, std::ranges::none_of> none_parse{};

}  // namespace tests
}  // namespace rp
