#include <string_view>

#include "parser.h"

using namespace rp::parser;

auto do_something = [](){return true;};

struct foo
{
    using match = literal<'a'>;
    static constexpr auto action = do_something;
};

int main() {
    using namespace std::string_view_literals;

    constexpr std::string_view a{"hello world"};
    static_assert(!match<foo>(a.begin(), a.end()));
}
