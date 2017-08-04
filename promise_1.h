// Copyright 2017 - Kirk Shoop
// see LICENSE

#pragma once

#include "single_async_subject.h"

namespace promise_1 {

template<typename T>
struct promise
{
    using result_type = std::decay_t<decltype(std::declval<single_async_subject<T>>().get_single_deferred())>;
    result_type result;
    lifetime<token_lifetime> token;

    promise(result_type r, lifetime<token_lifetime> t) : result(r), token(t) {}

    template<typename F>
    explicit promise(F&& f, ...) {
        single_async_subject<T> sub;
        result = sub.get_single_deferred();
        token = single_create([f](auto s){
            f([s](auto v){ s.value(v); }, [s](auto e){ s.error(e); });
        }) | 
        single_subscribe(sub.get_single());
    }
    template<typename F>
    promise(std::launch policy, F&& f) {
        single_async_subject<T> sub;
        result = sub.get_single_deferred();

        token = single_create([f](auto s){
                f([s](auto v){ s.value(v); }, [s](auto e){ s.error(e); });
            }) | 
            single_produce_on(policy) |
            single_subscribe(sub.get_single());
    }

    template<typename F>
    auto then(F&& f) const {
        using R = decltype(f(std::declval<T>()));
        single_async_subject<R> sub;
        auto l = result | 
            single_transform(std::forward<F>(f)) | 
            single_subscribe(sub.get_single());
        return promise<R>{sub.get_single_deferred(), l};
    }

    void cancel() const {
        token.stop();
    }

    void wait() {
        result | single_wait();
    }

};

}