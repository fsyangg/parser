#include <concepts>
#include <cstddef>
#include <string_view>
#include <tuple>
#include <vector>

#include "combinators.h"
#include "helpers.h"
#include "parsers.h"
#include "values.h"

namespace rp
{
namespace tests
{

using namespace rp::parser;

struct base
{
    static constexpr auto action = [](std::string_view sv) { return sv; };
};

struct value_base
{
    static constexpr auto action = [](auto&& v) { return v; };
};

// local classes can't have static data members, so star actions live in these bases
template <typename Elem>
struct count_base
{
    static constexpr auto action = [](std::vector<Elem>&& v) { return v.size(); };
};

struct two_counts_base
{
    using elems = std::vector<std::tuple<std::string_view>>;
    static constexpr auto action = [](elems&& a, elems&& c) { return std::pair{a.size(), c.size()}; };
};

struct last_pair_base
{
    using elems = std::vector<std::tuple<std::string_view, std::string_view>>;
    static constexpr auto action = [](elems&& v) { return v.back(); };
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
    static_assert(std::same_as<std::remove_cv_t<decltype(v)>, std::optional<int>>);
    static_assert(v.has_value() && *v == 42);
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
    static_assert(std::same_as<std::remove_cv_t<decltype(v)>, std::optional<unsigned int>>);
    static_assert(v.has_value() && *v == 42u);

    // a negative value for an unsigned type should fail constant evaluation (compile error):
    // struct neg : value_base
    // {
    //     using match = emit<seq<literal<'-'>, literal<'4'>, literal<'2'>>, unsigned int>;
    // };
    // constexpr std::string_view n{"-42"};
    // static_assert(parse_with_action<neg>(n.begin(), n.end()) == 0);
}

void test_star_value_type()
{
    using sv = std::string_view;

    static_assert(star<emit<literal<'a'>>>::value_type() == std::vector{^^std::vector<std::tuple<sv>>});
    static_assert(star<literal<'a'>>::value_type() == std::vector{^^std::vector<std::tuple<>>});
    static_assert(star<seq<emit<literal<'a'>>, emit<literal<'b'>, int>>>::value_type() ==
                  std::vector{^^std::vector<std::tuple<sv, int>>});
}

void test_star_match()
{
    struct prod
    {
        using match = star<literal<'a'>>;
    };

    static_assert(all_parse<prod>({"", "a", "aaaaaaaaaa"}));
    static_assert(none_parse<prod>({"b", "bbbbb", "aab"}));
}

void test_star_in_sequence()
{
    struct prod
    {
        using match = seq<star<literal<'a'>>, star<literal<'b'>>>;
    };

    static_assert(all_parse<prod>({"", "aa", "ab", "aab", "aabb", "bb"}));
    static_assert(none_parse<prod>({"bbaa", "abba", "aaaba"}));
}

void test_star_strings()
{
    using ab = seq<literal<'a'>, literal<'b'>>;

    struct prod
    {
        using match = star<ab>;
    };

    static_assert(all_parse<prod>({"", "ab", "ababababab"}));
    static_assert(none_parse<prod>({"a", "aa", "aab", "abb", "aba", "abaa"}));

    // a partial match of ab must be unwound so the following literal can use it
    struct then_a
    {
        using match = seq<star<ab>, literal<'a'>>;
    };

    static_assert(all_parse<then_a>({"a", "aba", "ababa"}));
    static_assert(none_parse<then_a>({"", "ab", "abab", "abaa"}));
}

void test_star_collects()
{
    struct prod : count_base<std::tuple<std::string_view>>
    {
        using match = star<emit<literal<'a'>>>;
    };

    constexpr std::string_view three{"aaa"};
    static_assert(*parse_with_action<prod>(three.begin(), three.end()) == 3);

    constexpr std::string_view none{""};
    static_assert(*parse_with_action<prod>(none.begin(), none.end()) == 0);
}

void test_star_counts_without_values()
{
    struct prod : count_base<std::tuple<>>
    {
        using match = star<literal<'a'>>;
    };

    constexpr std::string_view s{"aaaa"};
    static_assert(*parse_with_action<prod>(s.begin(), s.end()) == 4);
}

void test_star_multiple_in_sequence()
{
    struct prod : two_counts_base
    {
        using match = seq<star<emit<literal<'a'>>>, literal<'b'>, star<emit<literal<'c'>>>>;
    };

    constexpr std::string_view s{"aabccc"};
    static_assert(*parse_with_action<prod>(s.begin(), s.end()) == std::pair<std::size_t, std::size_t>{2, 3});
}

void test_star_tuple_elements()
{
    struct prod : last_pair_base
    {
        using match = star<seq<emit<literal<'a'>>, emit<literal<'b'>>>>;
    };

    constexpr std::string_view s{"abab"};
    constexpr auto last{*parse_with_action<prod>(s.begin(), s.end())};
    static_assert(std::get<0>(last) == "a" && std::get<1>(last) == "b");
}

}  // namespace tests
}  // namespace rp
