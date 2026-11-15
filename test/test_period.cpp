#include <gtest/gtest.h>

#include <string_view>

#include "parser.h"

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
