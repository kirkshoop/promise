// Copyright 2017 - Kirk Shoop
// see LICENSE

#pragma once

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
                if (!!expired) {
                    expired();
                }
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

template<typename T>
auto nest_lifetime(const lifetime<token_lifetime>& l, T& t) {
    auto aliased = std::shared_ptr<T>{l.i.l.lock(), std::addressof(t)};
    return aliased;
}

template<typename L>
struct unique_stop;

template<typename L>
struct unique_stop<lifetime<L>>
{
    ~unique_stop() { if (!!l ) { l->stop(); } }

    explicit unique_stop(const lifetime<L>& l) : l(std::addressof(l)) {}

    void dismiss() { l = nullptr; }

    const lifetime<L>* l = nullptr;
};

template<typename C>
struct unique_stop<single_context<C>>
{
    using L = decltype(std::declval<single_context<C>>().get_lifetime());
    L l;

    ~unique_stop() { if (!!lp) { lp->stop(); } }

    explicit unique_stop(const single_context<C>& c) : l(c.get_lifetime()), lp(std::addressof(l)) {}

    void dismiss() { lp = nullptr; }

    L* lp = nullptr;
};

struct cancellation_error : std::runtime_error
{
    template<class... AN>
    cancellation_error(AN&&... an) : std::runtime_error(std::forward<AN>(an)...) {} 
};

template<typename S>
struct single_enforcer;

template<typename S>
struct single_enforcer<single<S>>
{
    using this_type = single_enforcer<single<S>>;
    
    single_enforcer(single<S> s) : s(std::move(s)), canceled(true) {}
    single<S> s;
    lifetime<unique_lifetime> myl;
    lifetime<token_lifetime> l;
    mutable bool canceled;

    template<typename C>
    void start(C&& c) {
        l = c.get_lifetime();
        myl.i.set([this](){
            this->l.stop();
            if (canceled) {
                this->s.error(std::make_exception_ptr<cancellation_error>("canceled"));
            }
        });
        auto nested = nest_lifetime(l, *this);
        single_context<token_lifetime_context> myc{{make_token_lifetime(nested, myl)}};
        s.start(myc);
    }

    template<typename V>
    void value(V&& v) const {
        if (myl.is_stopped()) { return; }
        canceled = false;
        unique_stop<lifetime<token_lifetime>> g(l);
        try {
            s.value(std::forward<V>(v)); 
        } catch(...) { 
            this->error(std::current_exception());
        }
    }
    template<typename E>
    void error(E&& e) const {
        if (myl.is_stopped()) { return; }
        canceled = false;
        unique_stop<lifetime<token_lifetime>> g(l);
        try {
            s.error(std::forward<E>(e)); 
        } catch(...) {
            std::terminate();
        }
    }
};
