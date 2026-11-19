#include <utility>

namespace rp
{
namespace parser
{

template <char C>
struct literal
{
    template <typename It, typename Next, typename... Args>
    static constexpr auto parse(It& cur, It end, Next&& next, Args&&... args)
    {
        if (cur == end || *cur != C)
        {
            return false;
        }
        ++cur;
        // no change to args
        return next(cur, end, std::forward<Args>(args)...);
    }
};

struct period
{
    template <typename It, typename Next, typename... Args>
    static constexpr auto parse(It& cur, It end, Next&& next, Args&&... args)
    {
        if (cur == end)
        {
            return false;
        }
        ++cur;
        // no change to args
        return next(cur, end, std::forward<Args>(args)...);
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

template <typename... Ps>
struct seq;

template <typename First, typename... Rest>
struct seq<First, Rest...>
{
    template <typename It, typename Next, typename... Args>
    static constexpr auto parse(It& cur, It end, Next&& next, Args&&... args)
    {
        // construct the continuation for the first parser, which is a call to the second parser, etc.
        // use auto&&... vals instead of args so first parser can add when it calls continuation
        constexpr auto continuation = [&next](It& cur, It end, auto&&... vals)
        { return seq<Rest...>::parse(cur, end, next, std::forward<decltype(vals)>(vals)...); };

        return First::parse(cur, end, continuation, std::forward<Args>(args)...);
    }
};

template <>
struct seq<>
{
    template <typename It, typename Next, typename... Args>
    static constexpr auto parse(It& cur, It end, Next&& next, Args&&... args)
    {
        return next(cur, end, std::forward<Args>(args)...);
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

// need to aggregate values here somehow
template <typename P, typename It>
static constexpr auto match(It begin, It end)
{
    constexpr auto final_parser = [](It& cur, It end, auto&&... args) { return cur == end; };
    const auto res{P::match::parse(begin, end, final_parser)};
    return res;
}

}  // namespace parser
}  // namespace rp
