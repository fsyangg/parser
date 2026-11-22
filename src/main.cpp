#include <string_view>
#include <utility>

#include "combinators.h"
#include "parsers.h"
#include "values.h"

using namespace rp::parser;

struct foo
{
    using match = seq< emit< literal<'a'> >, emit< literal<'b'> >>;
    static constexpr auto action = [](std::string_view first, std::string_view second) {
        return std::make_pair(first, second);
    };
};

int main() {
    using namespace std::string_view_literals;

    constexpr std::string_view a{"ab"};
    constexpr auto s{parse_with_action<foo>(a.begin(), a.end())};
    static_assert(s.has_value());

    constexpr auto first = s->first;
    constexpr auto second = s->second;
    static_assert(first == "a" && second == "b");
}
