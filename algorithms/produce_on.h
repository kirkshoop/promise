namespace detail {

struct produce_on_async_context : token_lifetime_context
{
    explicit produce_on_async_context(const lifetime<token_lifetime>& l) : 
        token_lifetime_context(token_lifetime_context{l}) {}
};

template<typename S>
struct produce_on_async_state : std::enable_shared_from_this<produce_on_async_state<S>>
{
    produce_on_async_state(S s) : s(std::move(s)) {}
    S s;
    mutable lifetime<unique_lifetime> l;
    mutable std::future<void> result;
    mutable std::function<single_enforcer<S, single_context<produce_on_async_context>>()> out;

    template<typename Context>
    void start(Context&& c) const {
        l.i.set([]{c.get_lifetime();});
        single_context<produce_on_async_context> myc{produce_on_async_context{make_token_lifetime(this->shared_from_this(), this->l)}};
        out = [this, myc](){
            return single_enforcer<S, single_context<produce_on_async_context>>{this->s, myc};
        };
        s.start(myc);
    }
    template<typename V>
    void value(V&& v) const {
        out().value(std::forward<V>(v));
    }
    template<typename E>
    void error(E&& e) const {
        out().error(std::forward<E>(e));
    }
};

template<typename In>
struct produce_on_async_inner
{
    In in;
    std::launch policy;

    template<typename S, typename State = produce_on_async_state<std::decay_t<S>>>
    auto subscribe(S&& s) const {
        auto state = std::make_shared<State>(std::forward<S>(s));
        single<single_ptr<std::shared_ptr<State>>> out{single_ptr<std::shared_ptr<State>>{state}};
        state->result = std::async(policy, [out](In in) mutable {
            in.subscribe(std::move(out));
        }, in);
        return make_token_lifetime(state, state->l);
    }
};

}

auto single_produce_on(std::launch policy) {
    return [policy](auto in) { 
        using I = detail::produce_on_async_inner<std::decay_t<decltype(in)>>;
        return single_deferred<I>{I{in, policy}};
    };
}
