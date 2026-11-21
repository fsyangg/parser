A header-only PEG parser library, inspired by
[Boost.Spirit](https://www.boost.org/doc/libs/latest/libs/spirit/doc/x3/html/index.html),
the [PEGTL](https://github.com/taocpp/PEGTL), and [lexy](https://github.com/foonathan/lexy).

Example usage:

```cpp
using namespace rp::parser;

struct production
{
    using match = emit< seq< emit< literal<'a'> >, emit< literal<'b'> >, emit< literal<'c'> >>>;
    static constexpr auto action = [](std::string_view sv) {
        return sv;
    };
}

int main()
{
    // supports semantic actions, parse_with_action is constexpr
    constexpr std::string_view good{"abc"};
    constexpr auto val{parse_with_action<production>(good.begin(), good.end())};
    static_assert(val == "abc");

    // or just matching, match is also constexpr
    constexpr std::string_view bad{"cba"};
    static_assert(!match<production>(bad.begin(), bad.end()));
}
```

Supports compile-time parsing into structs (not yet) using C++26 reflection. I developed this using gcc16.2
(reflection supported since [16.1](https://gcc.gnu.org/gcc-16/changes.html#cxx)).

I have no clue if this works with Bloomberg's p2996 fork or any other compilers that are supposed to implement p2996,
as I have not tested it.
