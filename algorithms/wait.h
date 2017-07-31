// Copyright 2017 - Kirk Shoop
// see License.txt

#pragma once

namespace detail {

struct wait_state 
{
    mutable std::mutex lock;
    mutable std::condition_variable complete;
    mutable std::exception_ptr ep;

    template<typename Context>
    void start(Context&&) const {
    }
    template<typename V>
    void value(V&& ) const {
        complete.notify_one();
    }
    void error(std::exception_ptr e) const {
        ep = e;
        complete.notify_one();
    }
};

}

auto single_wait() {
    return [](auto in){
        detail::wait_state state{};

        in.subscribe(single<single_ptr<detail::wait_state*>>{single_ptr<detail::wait_state*>{std::addressof(state)}});

        std::unique_lock<std::mutex> guard(state.lock);
        state.complete.wait(guard);
        if (!!state.ep) { std::rethrow_exception(state.ep); }
    };
}
