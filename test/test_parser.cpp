#include <gtest/gtest.h>

#include <algorithm>
#include <string_view>

#include "parser.h"

TEST(Literal, Basic)
{
    using prod = parser::literal<'x'>;

    constexpr std::string_view wrong{"y"};
    EXPECT_FALSE(parser::parse<prod>(wrong.begin(), wrong.end()));

    constexpr std::string_view right{"x"};
    EXPECT_TRUE(parser::parse<prod>(right.begin(), right.end()));

    EXPECT_FALSE(parser::parse<prod>(right.begin(), right.begin()));
}

TEST(Period, Basic)
{
    using prod = parser::period;
    constexpr std::string_view s{"a"};
    constexpr auto b{parser::parse<prod>(s.begin(), s.end())};
    EXPECT_TRUE(b);
}

TEST(Period, Sequence)
{
    using prod = parser::seq<parser::period, parser::period>;
    constexpr std::string_view s{"aa"};
    constexpr auto b{parser::parse<prod>(s.begin(), s.end())};
    EXPECT_TRUE(b);

    constexpr std::string_view s2{"aaa"};
    constexpr auto b2{parser::parse<prod>(s2.begin(), s2.end())};
    EXPECT_FALSE(b2);

    constexpr std::string_view s3{"a"};
    constexpr auto b3{parser::parse<prod>(s3.begin(), s3.end())};
    EXPECT_FALSE(b3);
}

TEST(Seq, Basic)
{
    using prod = parser::seq<parser::literal<'a'>, parser::literal<'b'>, parser::literal<'c'>>;

    constexpr std::string_view good{"abc"};
    EXPECT_TRUE(parser::parse<prod>(good.begin(), good.end()));

    constexpr std::string_view bad{"abd"};
    EXPECT_FALSE(parser::parse<prod>(bad.begin(), bad.end()));
}

TEST(Seq, Repeat)
{
    using prod = parser::seq<parser::literal<'a'>, parser::literal<'a'>, parser::literal<'a'>>;

    constexpr std::string_view bad{"bbb"};
    EXPECT_FALSE(parser::parse<prod>(bad.begin(), bad.end()));

    constexpr std::string_view good{"aaa"};
    EXPECT_TRUE(parser::parse<prod>(good.begin(), good.end()));
}

TEST(Seq, Empty)
{
    using prod = parser::seq<>;
    constexpr std::string_view good{"aaa"};
    EXPECT_TRUE(parser::parse<prod>(good.begin(), good.begin()));
    EXPECT_FALSE(parser::parse<prod>(good.begin(), good.end()));
}

TEST(Seq, Lengths)
{
    using two = parser::seq<parser::literal<'a'>, parser::literal<'a'>>;

    constexpr std::string_view too_short{"a"};
    EXPECT_FALSE(parser::parse<two>(too_short.begin(), too_short.end()));

    constexpr std::string_view too_long{"aaa"};
    EXPECT_FALSE(parser::parse<two>(too_long.begin(), too_long.end()));
}

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
    constexpr auto b{
        std::ranges::all_of(strs, [](auto s) { return parser::parse<prod>(s.begin(), s.end()); })};
    static_assert(b);
    EXPECT_TRUE(b);

    constexpr std::array<std::string_view, 6> bad_strs{"bbaa", "abba", "aaaba"};
    constexpr auto b2{std::ranges::all_of(
        bad_strs, [](auto s) { return parser::parse<prod>(s.begin(), s.end()); })};
    static_assert(!b2);
    EXPECT_FALSE(b2);
}

TEST(Star, Strings)
{
    using ab = parser::seq<parser::literal<'a'>, parser::literal<'b'>>;
    using prod = parser::star<ab>;

    constexpr auto good_strs = std::to_array<std::string_view>({"", "ab", "ababababab"});
    constexpr auto bad_strs =
        std::to_array<std::string_view>({"a", "aa", "aab", "abb", "aba", "abaa"});
    constexpr auto good{std::ranges::all_of(
        good_strs, [](auto s) { return parser::parse<prod>(s.begin(), s.end()); })};
    static_assert(good);
    EXPECT_TRUE(good);

    constexpr auto bad{std::ranges::all_of(
        bad_strs, [](auto s) { return parser::parse<prod>(s.begin(), s.end()); })};
    static_assert(!bad);
    EXPECT_FALSE(bad);
}

TEST(Plus, Basic)
{
    using aplus = parser::plus<parser::literal<'a'>>;
    using bplus = parser::plus<parser::literal<'b'>>;
    using prod = parser::seq<aplus, bplus>;

    constexpr auto good_strs =
        std::to_array<std::string_view>({"ab", "aaaaaaab", "abbbbbbbbb", "aabb"});
    constexpr auto b{std::ranges::all_of(
        good_strs, [](auto s) { return parser::parse<prod>(s.begin(), s.end()); })};
    static_assert(b);
    EXPECT_TRUE(b);

    constexpr auto bad_strs =
        std::to_array<std::string_view>({"a", "b", "aaaaa", "bbbbb", "ba", "baa"});
    constexpr auto b2{std::ranges::all_of(
        bad_strs, [](auto s) { return parser::parse<prod>(s.begin(), s.end()); })};
    static_assert(!b2);
    EXPECT_FALSE(b2);
}

TEST(Plus, Strings)
{
    using ab = parser::seq<parser::literal<'a'>, parser::literal<'b'>>;
    using prod = parser::plus<ab>;

    constexpr auto good_strs =
        std::to_array<std::string_view>({"ab", "ababab", "abababababababab"});
    constexpr auto b{std::ranges::all_of(
        good_strs, [](auto s) { return parser::parse<prod>(s.begin(), s.end()); })};
    static_assert(b);
    EXPECT_TRUE(b);

    constexpr auto bad_strs = std::to_array<std::string_view>({"", "a", "aa", "ababa", "abb"});
    constexpr auto b2{std::ranges::all_of(
        bad_strs, [](auto s) { return parser::parse<prod>(s.begin(), s.end()); })};
    static_assert(!b2);
    EXPECT_FALSE(b2);
}
