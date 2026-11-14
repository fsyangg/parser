#include <gtest/gtest.h>

#include <string_view>

#include "parser.h"

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
