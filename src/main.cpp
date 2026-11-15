#include <string_view>

#include "parser.h"

int main() {
    using namespace std::string_view_literals;
    using namespace parser;

    constexpr auto s{"y"sv};
    constexpr auto b{parser::parse<parser::literal<'x'>>(s.begin(), s.end())};
    static_assert(!b);

    using empty = parser::seq<>;
    constexpr auto empty_res{parser::parse<empty>(s.begin(), s.end())};
    constexpr auto nonempty{parser::parse<empty>(s.begin(), s.end())};
}
