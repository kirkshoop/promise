// Copyright 2017 - Kirk Shoop
// see LICENSE

#pragma once

namespace detail {

template<typename S>
struct create_state : std::enable_shared_from_this<create_state<S>>
{
    explicit create_state(S s) : 
        l(), 
        out(std::move(s)) {}
    lifetime<unique_lifetime> l;
    single_enforcer<S> out;
};

template<typename P>
struct create_inner
{
    P p;

    template<typename S>
    auto subscribe(S&& s) const {
        using enforcer = single_enforcer<std::decay_t<S>>;
        auto state = std::make_shared<create_state<std::decay_t<S>>>(std::forward<S>(s));
        auto c = single_context<token_lifetime_context>{{make_token_lifetime(state, state->l)}};
        state->out.start(c);
        single<single_ptr<std::shared_ptr<enforcer>>> out{single_ptr<std::shared_ptr<enforcer>>{nest_lifetime(c.get_lifetime(), state->out)}};
        p(std::move(out));
        return c.get_lifetime();
    }
};

}

template<typename P, typename I = detail::create_inner<std::decay_t<P>>>
auto single_create(P&& p) {
    return single_deferred<I>{I{std::forward<P>(p)}};
}
