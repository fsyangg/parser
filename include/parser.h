namespace rp
{
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
        if constexpr (sizeof...(Productions) == 0)
        {
            return true;
        }

        const auto copy = cur;
        if ((Productions::match(cur, end) && ...))
        {
            return true;
        }
        cur = copy;
        return false;
    }
};

template <typename First, typename... Rest>
struct branch
{
    template <typename Iterator>
    static constexpr auto match(Iterator& cur, Iterator end)
    {
        const auto copy = cur;
        if (First::match(cur, end))
        {
            return true;
        }
        cur = copy;
        return (Rest::match(cur, end) || ...);
    }
};

template <typename First>
struct branch<First>
{
    template <typename Iterator>
    static constexpr auto match(Iterator& cur, Iterator end)
    {
        // do we need to unwind here, not sure
        return First::match(cur, end);
    }
};

template <typename Production, typename Iterator>
static constexpr auto parse(Iterator begin, Iterator end)
{
    auto res{Production::match(begin, end)};
    return res && begin == end;
}

}  // namespace parser
}  // namespace rp
