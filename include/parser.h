namespace parser
{

template <char C>
struct literal
{
    template <typename Iterator>
    static constexpr auto match(Iterator& cur, Iterator end)
    {
        if (cur == end || *cur != C)
        {
            return false;
        }
        ++cur;
        return true;
    }
};

struct period
{
    template <typename Iterator>
    static constexpr auto match(Iterator& cur, Iterator end)
    {
        if (cur == end)
        {
            return false;
        }
        ++cur;
        return true;
    }
};

template <typename Production>
struct star
{
    template <typename Iterator>
    static constexpr auto match(Iterator& cur, Iterator end)
    {
        if (cur == end || !Production::match(cur, end))
        {
            return true;
        }
        return match(cur, end);
    }
};

template <typename Production>
struct plus
{
    template <typename Iterator>
    static constexpr auto match(Iterator& cur, Iterator end)
    {
        return Production::match(cur, end) && star<Production>::match(cur, end);
    }
};

template <typename... Productions>
struct seq
{
    template <typename Iterator>
    static constexpr auto match(Iterator& cur, Iterator end)
    {
        return (cur == end || (Productions::match(cur, end) && ...));
    }
};

template <>
struct seq<>
{
    template <typename Iterator>
    static constexpr auto match(Iterator& cur, Iterator end)
    {
        return true;
    }
};

template <typename Production, typename Iterator>
static constexpr auto parse(Iterator begin, Iterator end)
{
    auto res{Production::match(begin, end)};
    return res && begin == end;
}

}  // namespace parser
