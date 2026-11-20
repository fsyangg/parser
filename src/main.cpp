#include <string_view>

#include "parser.h"

using namespace rp::parser;

struct foo
{
    using match = emit<literal<'a'>>;
    static constexpr auto action = [](std::string_view&& sv) {
        return sv;
    };
};

int main() {
    using namespace std::string_view_literals;

    constexpr std::string_view a{"a"};
    constexpr auto s{parse_with_action<foo>(a.begin(), a.end())};
    static_assert(s == "a");
}
