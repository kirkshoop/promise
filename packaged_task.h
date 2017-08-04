// Copyright 2017 - Kirk Shoop
// see License.txt

#pragma once

#include <tuple>
#include <experimental/tuple>

#include "single_async_subject.h"

namespace packaged_task {

template<typename T>
struct future
{
    using result_type = std::decay_t<decltype(std::declval<single_async_subject<T>>().get_single_deferred())>;
    result_type result;
    lifetime<token_lifetime> token;

    future(result_type r, lifetime<token_lifetime> t) : result(r), token(t) {}

    template<typename F>
    auto then(F&& f) const {
        using R = decltype(f(std::declval<T>()));
        single_async_subject<R> sub;
        auto l = result | 
            single_transform(std::forward<F>(f)) | 
            single_subscribe(sub.get_single());
        return future<R>{sub.get_single_deferred(), l};
    }

    void cancel() const {
        token.stop();
    }

    void wait() {
        result | single_wait();
    }
};

template<typename F>
struct packaged_task
{
    std::function<F> f;
    using R = typename std::function<F>::result_type;
    single_async_subject<R> sub;

    template<typename T>
    explicit packaged_task(T&& t, ...) : f(std::forward<T>(t)) {
    }

    template<class... AN>
    void operator()(AN&&... an){
        if (sub.get_lifetime().is_stopped()) { return; }

        auto fn = f;
        auto args = std::make_tuple(std::forward<AN>(an)...);
        single_create([fn, args](auto s){
            s.value(std::experimental::apply(fn, args));
        }) |
        single_subscribe(sub.get_single());
    }

    future<R> get_future() {
        return {sub.get_single_deferred(), sub.get_lifetime()};
    }

    void reset() {
        sub = single_async_subject<R>{};
    }
};

}
