// Copyright 2017 - Kirk Shoop
// see LICENSE

#pragma once

namespace detail {

template<typename Tap, typename S>
struct tap_state
{
    tap_state(Tap tap, S s) : tap(std::move(tap)), s(std::move(s)) {}
    Tap tap;
    S s;

    template<typename Context>
    void start(Context&& c) const {
        tap.start(const_cast<const Context&>(c));
        s.start(std::forward<Context>(c));
    }
    template<typename V>
    void value(V&& v) const {
        tap.value(const_cast<const V&>(v));
        s.value(std::forward<V>(v));
    }
    template<typename E>
    void error(E&& e) const {
        tap.error(const_cast<const E&>(e));
        s.error(std::forward<E>(e));
    }
};

struct tap_context : token_lifetime_context
{
    explicit tap_context(const lifetime<token_lifetime>& l) : 
        token_lifetime_context(token_lifetime_context{l}) {}
};

template<typename In, typename Tap>
struct tap_inner
{
    In in;
    Tap tap;

    template<typename S, typename State = tap_state<Tap, std::decay_t<S>>>
    auto subscribe(S&& s) const {
        return in.subscribe(single<State>{State{tap, std::forward<S>(s)}});
    }
};

}

template<typename S>
auto single_tap(S&& s) {
    return [s](auto in) { 
        using I = detail::tap_inner<std::decay_t<decltype(in)>, std::decay_t<S>>;
        return single_deferred<I>{I{in, s}};
    };
}
