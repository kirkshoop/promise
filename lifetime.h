// Copyright 2017 - Kirk Shoop
// see LICENSE

#pragma once

template<typename L>
struct unique_stop;

template<typename L>
struct unique_stop<lifetime<L>>
{
    ~unique_stop() { if (!!l) { l->stop(); } }

    explicit unique_stop(const lifetime<L>& l) : l(std::addressof(l)) {}

    void dismiss() { l = nullptr; }

    const lifetime<L>* l = nullptr;
};

template<typename C>
struct unique_stop<single_context<C>>
{
    using L = decltype(std::addressof(std::declval<single_context<C>>().get_lifetime()));

    ~unique_stop() { if (!!l) { l->stop(); } }

    explicit unique_stop(const single_context<C>& c) : l(std::addressof(c.get_lifetime())) {}

    void dismiss() { l = nullptr; }

    L l = nullptr;
};

template<typename S, typename C>
struct single_enforcer;

template<typename S, typename C>
struct single_enforcer<single<S>, single_context<C>>
{
    single_enforcer(single<S> s, single_context<C> c) : s(std::move(s)), c(std::move(c)) {
        this->s.start(this->c);
    }
    single<S> s;
    single_context<C> c;
    template<typename V>
    void value(V&& v) const {
        if (c.get_lifetime().is_stopped()) { return; }
        unique_stop<single_context<C>> g(c);
        s.value(std::forward<V>(v)); 
    }
    template<typename E>
    void error(E&& e) const {
        if (c.get_lifetime().is_stopped()) { return; }
        unique_stop<single_context<C>> g(c);
        s.error(std::forward<E>(e)); 
    }
};

struct unique_lifetime
{
    unique_lifetime() = default;
    unique_lifetime(const unique_lifetime&) = delete;
    unique_lifetime& operator=(const unique_lifetime&) = delete;
    unique_lifetime(unique_lifetime&&) = default;
    unique_lifetime& operator=(unique_lifetime&&) = default;

    mutable std::mutex lock = {};
    mutable bool stopped = false;
    mutable std::function<void()> dostop = {};

    bool is_stopped() const {
        std::unique_lock<std::mutex> guard(lock);
        return stopped;
    }
    void stop() const {
        std::unique_lock<std::mutex> guard(lock);
        if (!stopped) {
            stopped = true;
            if (!!dostop) {
                auto expired = std::move(dostop);
                dostop = nullptr;
                guard.unlock();
                dostop();
            }
        }
    }
    void set(std::function<void()> s) const {
        std::unique_lock<std::mutex> guard(lock);
        if (stopped) {
            s();
        } else {
            dostop = std::move(s);
        }
    }
};

struct token_lifetime
{
    token_lifetime() = default;
    explicit token_lifetime(const std::shared_ptr<lifetime<unique_lifetime>>& l) : l(l) {}

    std::weak_ptr<lifetime<unique_lifetime>> l;

    bool is_stopped() const { auto s = l.lock(); return !s || s->is_stopped(); }
    void stop() const { auto s = l.lock(); if (!s) {return;} s->stop(); }
};

struct unique_lifetime_context
{
    lifetime<unique_lifetime> l;

    const auto& get_lifetime() const { return l; }
};

struct token_lifetime_context
{
    lifetime<token_lifetime> l;

    const auto& get_lifetime() const { return l; }
};

template<typename T, typename L>
lifetime<token_lifetime> make_token_lifetime(const std::shared_ptr<T>& o, lifetime<L>& l) {
    auto aliased = std::shared_ptr<lifetime<L>>{o, std::addressof(l)};
    return lifetime<token_lifetime>{token_lifetime{aliased}};
}
