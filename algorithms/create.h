// Copyright 2017 - Kirk Shoop
// see License.txt

#pragma once

namespace detail {

struct create_state
{
    lifetime<unique_lifetime> l;
};

struct create_context : token_lifetime_context
{
    std::shared_ptr<create_state> state;

    create_context(const std::shared_ptr<create_state>& state, const lifetime<token_lifetime>& l) : 
        token_lifetime_context(token_lifetime_context{l}), 
        state(state) {}
};

template<typename P>
struct create_inner
{
    P p;

    template<typename S>
    auto subscribe(S&& s) const {
        auto state = std::make_shared<create_state>();
        single_context<create_context> c{create_context{state, make_token_lifetime(state, state->l)}};
        single_enforcer<std::decay_t<S>, single_context<create_context>> out{
            std::forward<S>(s), c
        };
        p(out);
        return c;
    }
};

}

template<typename P, typename I = detail::create_inner<std::decay_t<P>>>
auto single_create(P&& p) {
    return single_deferred<I>{I{std::forward<P>(p)}};
}
