#include <string_view>

#include "parser.h"

namespace rp
{
namespace tests
{

using namespace rp::parser;

struct base
{
    static constexpr auto action = [](std::string_view&& sv) {
        return sv;
    };
};

void test_emit_literal()
{
    struct prod : base
    {
        using match = emit<literal<'x'>>;
    };

    constexpr std::string_view s{"x"};
    static_assert(parse_with_action<prod>(s.begin(), s.end()) == "x");
}

void test_emit_seq()
{
    struct prod : base
    {
        using match = emit<seq<literal<'a'>, literal<'b'>, literal<'c'>>>;
    };

    constexpr std::string_view s{"abc"};
    static_assert(parse_with_action<prod>(s.begin(), s.end()) == "abc");
}

}  // namespace tests
}  // namespace rp
