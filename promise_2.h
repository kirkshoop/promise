// Copyright 2017 - Kirk Shoop
// see LICENSE

#pragma once

#include "single_async_subject.h"

namespace promise_2 {

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

template<typename T>
struct promise
{
    single_async_subject<T> sub;
    lifetime<token_lifetime> token;

    future<T> get_future() const {
        return {sub.get_single_deferred(), sub.get_lifetime()};
    }

    void set_value(T t) {
        sub.get_single().value(std::move(t));
    }
    void set_exception(std::exception_ptr ep) {
        sub.get_single().error(ep);
    }
};

}