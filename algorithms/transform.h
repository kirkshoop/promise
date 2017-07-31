// Copyright 2017 - Kirk Shoop
// see License.txt

#pragma once

namespace detail {

template<typename F, typename S>
struct transform_state
{
    transform_state(F f, S s) : f(std::move(f)), s(std::move(s)) {}
    F f;
    S s;

    template<typename Context>
    void start(Context&& c) const {
        s.start(std::forward<Context>(c));
    }
    template<typename V>
    void value(V&& v) const {
        s.value(f(std::forward<V>(v)));
    }
    template<typename E>
    void error(E&& e) const {
        s.error(std::forward<E>(e));
    }
};

struct transform_context : token_lifetime_context
{
    explicit transform_context(const lifetime<token_lifetime>& l) : 
        token_lifetime_context(token_lifetime_context{l}) {}
};

template<typename In, typename F>
struct transform_inner
{
    In in;
    F f;

    template<typename S, typename State = transform_state<F, std::decay_t<S>>>
    auto subscribe(S&& s) const {
        return in.subscribe(single<State>{State{f, std::forward<S>(s)}});
    }
};

}

template<typename F>
auto single_transform(F&& f) {
    return [f](auto in) { 
        using I = detail::transform_inner<std::decay_t<decltype(in)>, std::decay_t<F>>;
        return single_deferred<I>{I{in, f}};
    };
}
