#include <gtest/gtest.h>
#include <string_view>

#include "parser.h"

TEST(Literal, Basic) {
    using prod = parser::literal<'x'>;

    constexpr std::string_view wrong{"y"};
    EXPECT_FALSE(parser::parse<prod>(wrong.begin(), wrong.end()));

    constexpr std::string_view right{"x"};
    EXPECT_TRUE(parser::parse<prod>(right.begin(), right.end()));

    EXPECT_FALSE(parser::parse<prod>(right.begin(), right.begin()));
}
