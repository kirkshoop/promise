// Copyright 2017 - Kirk Shoop
// see LICENSE

#pragma once

namespace detail {

template<typename T>
struct single_async_subject_state : std::enable_shared_from_this<single_async_subject_state<T>>
{
    using this_type = single_async_subject_state<T>;

    mutable T t;
    mutable std::exception_ptr ep;
    lifetime<unique_lifetime> l;
    std::function<void(const this_type&)> out;

    template<typename Context>
    void start(Context&& c) const {
        l.i.set(c.get_lifetime());
        }
    void value(T t) const {
        if (l.is_stopped()) { return; }
        l.stop();
        this->t = std::move(t);
        if (!!out) { out(*this); }
    }
    void error(std::exception_ptr ep) const {
        if (l.is_stopped()) { return; }
        l.stop();
        this->ep = ep;
        if (!!out) { out(*this); }
    }

    template<typename S>
    auto subscribe(S&& s) { 
        auto f = [s](const this_type& that){
            if (!!that.ep) {
                s.error(that.ep);
            } else {
                s.value(that.t);
            }
        };
        if (l.is_stopped()) {
            f(*this);
        } else {
            out = f;
        }
        return make_token_lifetime(this->shared_from_this(), l);
    }
};

}

template<typename T>
struct single_async_subject
{
    using state_type = std::shared_ptr<detail::single_async_subject_state<T>>;
    state_type state = std::make_shared<detail::single_async_subject_state<T>>();

    lifetime<token_lifetime> get_lifetime() const {
        return {make_token_lifetime(state, state->l)};
    }

    single<single_ptr<state_type>> get_single() const {
        return {{state}};
    }

    single_deferred<single_deferred_ptr<state_type>> get_single_deferred() const {
        return {{state}};
    }
};
