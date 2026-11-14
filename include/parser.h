namespace parser
{

template <char C>
struct literal {
    template <typename Iterator>
    static constexpr auto match(Iterator cur, Iterator end) {
        if (cur == end || *cur != C) {
            return false;
        }
        ++cur;
        return true;
    }
};

template <typename... Productions>
struct seq {
    template <typename Input>
    static constexpr auto match(Input in) {
        return (Productions::match(in) && ...);
    }
};

template <typename Production, typename Iterator>
static constexpr auto parse(Iterator begin, Iterator end) {
    return Production::match(begin, end);
}

}  // namespace parser
