#include <algorithm>
#include <initializer_list>
#include <string_view>

#include "parser.h"

namespace rp
{
namespace tests
{

using namespace rp::parser;

template <typename Prod, auto Func>
struct parse_func
{
    static constexpr bool operator()(std::initializer_list<std::string_view> strs)
    {
        return Func(strs, [](auto s) { return parse<Prod>(s.begin(), s.end()); });
    }
};

template <typename Prod>
constexpr parse_func<Prod, std::ranges::all_of> all_parse{};

template <typename Prod>
constexpr parse_func<Prod, std::ranges::none_of> none_parse{};

void test_literal()
{
    using prod = literal<'x'>;

    static_assert(all_parse<prod>({"x"}));
    static_assert(!all_parse<prod>({"y"}));
}

void test_period_basic() { static_assert(all_parse<period>({"a"})); }

void test_period_sequence()
{
    using prod = seq<period, period>;

    static_assert(all_parse<prod>({"aa"}));
    static_assert(none_parse<prod>({"aaa", "a"}));
}

void test_seq_basic()
{
    using prod = seq<literal<'a'>, literal<'b'>, literal<'c'>>;

    static_assert(all_parse<prod>({"abc"}));
    static_assert(none_parse<prod>({"abd"}));
}

void test_seq_repeat()
{
    using prod = seq<literal<'a'>, literal<'a'>, literal<'a'>>;

    static_assert(all_parse<prod>({"aaa"}));
    static_assert(none_parse<prod>({"bbb"}));
}

void test_seq_empty()
{
    using prod = seq<>;

    static_assert(all_parse<prod>({""}));
    static_assert(none_parse<prod>({"aaa"}));
}

void test_seq_lengths()
{
    using prod = seq<literal<'a'>, literal<'a'>>;

    static_assert(none_parse<prod>({"a", "aaa"}));
    static_assert(none_parse<prod>({""}));
}

void test_star_basic()
{
    using prod = star<literal<'a'>>;

    static_assert(all_parse<prod>({"aaaaaaaaaa"}));
    static_assert(none_parse<prod>({"bbbbb"}));
}

void test_star_in_sequence()
{
    using astar = star<literal<'a'>>;
    using bstar = star<literal<'b'>>;
    using prod = seq<astar, bstar>;

    static_assert(all_parse<prod>({"aa", "ab", "aab", "aabb", "bb"}));
    static_assert(none_parse<prod>({"bbaa", "abba", "aaaba"}));
}

void test_star_strings()
{
    using ab = seq<literal<'a'>, literal<'b'>>;
    using prod = star<ab>;

    static_assert(all_parse<prod>({"", "ab", "ababababab"}));
    static_assert(none_parse<prod>({"a", "aa", "aab", "abb", "aba", "abaa"}));
}

void test_plus_basic()
{
    using aplus = plus<literal<'a'>>;
    using bplus = plus<literal<'b'>>;
    using prod = seq<aplus, bplus>;

    static_assert(all_parse<prod>({"ab", "aaaaaaab", "abbbbbbbbb", "aabb"}));
    static_assert(none_parse<prod>({"a", "b", "aaaaa", "bbbbb", "ba", "baa"}));
}

void test_plus_strings()
{
    using ab = seq<literal<'a'>, literal<'b'>>;
    using prod = plus<ab>;

    static_assert(all_parse<prod>({"ab", "ababab", "abababababababab"}));
    static_assert(none_parse<prod>({"", "a", "aa", "ababa", "abb"}));
}

void test_branch_basic()
{
    using prod = branch<literal<'a'>, literal<'b'>, literal<'c'>>;

    static_assert(all_parse<prod>({"a", "b", "c"}));
    static_assert(none_parse<prod>({"", "d", "ab", "ca", "aa"}));
}

void test_branch_single()
{
    using prod = branch<literal<'a'>>;

    static_assert(all_parse<prod>({"a"}));
    static_assert(none_parse<prod>({"", "b", "aa"}));
}

void test_branch_backtrack()
{
    using a = literal<'a'>;
    using ab = seq<a, literal<'b'>>;
    using ac = seq<a, literal<'c'>>;
    using ad = seq<a, literal<'d'>>;
    using prod = branch<ab, ac, ad>;

    static_assert(all_parse<prod>({"ab", "ac", "ad"}));
    static_assert(none_parse<prod>({"a", "ae", "ba", "abc", "aab"}));
}

void test_branch_ordered_choice()
{
    using a = literal<'a'>;
    using ab = seq<a, literal<'b'>>;

    // first matching alternative wins; no backtracking into a later one afterwards
    using short_first = branch<a, ab>;
    static_assert(none_parse<short_first>({"ab"}));

    using long_first = branch<ab, a>;
    static_assert(all_parse<long_first>({"a", "ab"}));
}

void test_branch_in_sequence()
{
    using ab = branch<literal<'a'>, literal<'b'>>;
    using prod = seq<ab, literal<'c'>, ab>;

    static_assert(all_parse<prod>({"aca", "acb", "bca", "bcb"}));
    static_assert(none_parse<prod>({"ac", "cca", "abc", "acc", "acab"}));
}

void test_branch_repeated()
{
    using ab = branch<literal<'a'>, literal<'b'>>;
    using star_prod = star<ab>;
    using plus_prod = plus<ab>;

    static_assert(all_parse<star_prod>({"", "a", "b", "abba", "bbbbaaaab"}));
    static_assert(none_parse<star_prod>({"c", "abc", "cab", "abacab"}));

    static_assert(all_parse<plus_prod>({"a", "b", "abba", "bbbbaaaab"}));
    static_assert(none_parse<plus_prod>({"", "c", "abc", "cab", "abacab"}));
}

void test_branch_nested()
{
    using a = literal<'a'>;
    using b = literal<'b'>;
    using c = literal<'c'>;
    using prod = branch<branch<a, b>, seq<c, branch<a, b>>>;

    static_assert(all_parse<prod>({"a", "b", "ca", "cb"}));
    static_assert(none_parse<prod>({"c", "cc", "ac", "cab"}));
}

}  // namespace tests
}  // namespace rp
