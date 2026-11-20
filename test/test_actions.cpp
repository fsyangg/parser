#include <concepts>
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

struct value_base
{
    static constexpr auto action = [](auto&& v) { return v; };
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

void test_emit_int_unsigned_digits()
{
    struct prod : value_base
    {
        using match = emit<seq<literal<'4'>, literal<'2'>>, int>;
    };

    constexpr std::string_view s{"42"};
    constexpr auto v{parse_with_action<prod>(s.begin(), s.end())};
    static_assert(std::same_as<std::remove_cv_t<decltype(v)>, int>);
    static_assert(v == 42);
}

void test_emit_int_signed()
{
    struct neg : value_base
    {
        using match = emit<seq<literal<'-'>, literal<'4'>, literal<'2'>>, int>;
    };
    struct pos : value_base
    {
        using match = emit<seq<literal<'+'>, literal<'4'>, literal<'2'>>, int>;
    };

    constexpr std::string_view n{"-42"};
    static_assert(parse_with_action<neg>(n.begin(), n.end()) == -42);

    constexpr std::string_view p{"+42"};
    static_assert(parse_with_action<pos>(p.begin(), p.end()) == 42);
}

void test_emit_unsigned_signed()
{
    struct pos : value_base
    {
        using match = emit<seq<literal<'+'>, literal<'4'>, literal<'2'>>, unsigned int>;
    };

    constexpr std::string_view p{"+42"};
    constexpr auto v{parse_with_action<pos>(p.begin(), p.end())};
    static_assert(std::same_as<std::remove_cv_t<decltype(v)>, unsigned int>);
    static_assert(v == 42u);

    // a negative value for an unsigned type should fail constant evaluation (compile error):
    // struct neg : value_base
    // {
    //     using match = emit<seq<literal<'-'>, literal<'4'>, literal<'2'>>, unsigned int>;
    // };
    // constexpr std::string_view n{"-42"};
    // static_assert(parse_with_action<neg>(n.begin(), n.end()) == 0);
}

}  // namespace tests
}  // namespace rp
