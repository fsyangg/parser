#include <gtest/gtest.h>
#include <string_view>
#include <algorithm>

#include "parser.h"

TEST(Star, Basic)
{
    using prod = parser::star<parser::literal<'a'>>;
    constexpr std::string_view repeat{"aaaaaaaaaa"};
    constexpr auto repeat_res{parser::parse<prod>(repeat.begin(), repeat.end())};
    static_assert(repeat_res);
    EXPECT_TRUE(repeat_res);

    constexpr std::string_view none{"bbbbb"};
    constexpr auto none_res{parser::parse<prod>(none.begin(), none.end())};
    static_assert(!none_res);
    EXPECT_FALSE(none_res);
}

TEST(Star, InSequence)
{
    using astar = parser::star<parser::literal<'a'>>;
    using bstar = parser::star<parser::literal<'b'>>;
    using prod = parser::seq<astar, bstar>;

    constexpr std::array<std::string_view, 6> strs{"aa", "ab", "aab", "aabb", "bb"};
    constexpr auto b{std::ranges::all_of(strs, [](auto s) { return parser::parse<prod>(s.begin(), s.end()); })};
    static_assert(b);
    EXPECT_TRUE(b);

    constexpr std::array<std::string_view, 6> bad_strs{"bbaa", "abba", "aaaba"};
    constexpr auto b2{std::ranges::all_of(bad_strs, [](auto s) { return parser::parse<prod>(s.begin(), s.end()); })};
    static_assert(!b2);
    EXPECT_FALSE(b2);
}
