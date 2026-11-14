namespace parser
{

template <char C>
struct literal {
    template <typename Input>
    static constexpr auto match(Input in) {
        return in[0] == C;
    }
};

template <typename Production, typename Input>
static constexpr auto parse(Input in) {
    return Production::match(in);
}

}  // namespace parser
