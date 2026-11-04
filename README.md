A header-only PEG parser library, inspired by [Boost.Spirit](https://www.boost.org/doc/libs/latest/libs/spirit/doc/x3/html/index.html)
and the [PEGTL](https://github.com/taocpp/PEGTL).

Supports compile-time parsing into structs using C++26 reflection. I developed this using gcc16.2
(reflection supported since [16.1](https://gcc.gnu.org/gcc-16/changes.html#cxx)).

I have no clue if this works with Bloomberg's p2996 fork or any other compilers that are supposed to implement p2996,
as I have not tested it.
