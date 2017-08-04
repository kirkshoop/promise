// Copyright 2017 - Kirk Shoop
// see LICENSE

#pragma once

#include <exception>
#include <new>
#include <type_traits>
#include <thread>
#include <future>
#include <chrono>

using namespace std::literals;

template<typename Inner>
struct lifetime
{
    Inner i;
    bool is_stopped() const { return i.is_stopped(); }
    void stop() const { i.stop(); }
    void operator()() const { i.stop(); }
};

template<typename InnerPtr>
struct lifetime_ptr
{
    InnerPtr i;
    bool is_stopped() const { return i->is_stopped(); }
    void stop() const { i->stop(); }
    void operator()() const { i->stop(); }
};

template<typename Inner>
struct single_context
{
    Inner i;
    auto get_lifetime() const { return i.get_lifetime(); }
};

template<typename InnerPtr>
struct single_context_ptr
{
    InnerPtr i;
    auto get_lifetime() const { return i->get_lifetime(); }
};

template<typename Inner>
struct single
{
    Inner i;
    template<typename Context>
    void start(Context&& c) const {
        i.start(std::forward<Context>(c));
    }
    template<typename V>
    void value(V&& v) const {
        i.value(std::forward<V>(v));
    }
    template<typename E>
    void error(E&& e) const {
        i.error(std::forward<E>(e));
    }
};

template<typename InnerPtr>
struct single_ptr
{
    InnerPtr i;
    template<typename Context>
    void start(Context&& c) const {
        i->start(std::forward<Context>(c));
    }
    template<typename V>
    void value(V&& v) const {
        i->value(std::forward<V>(v));
    }
    template<typename E>
    void error(E&& e) const {
        i->error(std::forward<E>(e));
    }
};

template<typename Inner>
struct single_deferred
{
    Inner i;
    template<typename S>
    auto subscribe(S&& s) const { 
        return i.subscribe(std::forward<S>(s)); 
    }
};

template<typename Inner>
auto single_subscribe(single<Inner> s) {
    return [s](auto in){
        return in.subscribe(s);
    };
}

std::ostream& operator<< (std::ostream& o, std::exception_ptr ep) {
    std::string s;
    try { std::rethrow_exception(ep); }
    catch(const std::exception& e) { s = e.what(); }
    catch(...) { s = "<not derived from std::exception>"; }
    return o << s;
}


struct single_ostream
{
    explicit single_ostream(std::ostream& o) : out(o) {}

    std::ostream& out;
    
    template<typename Context>
    void start(Context&&) const {}
    template<typename V>
    void value(V&& v) const {
        out << std::forward<V>(v) << std::endl;
    }
    template<typename E>
    void error(E&& e) const {
        out << std::forward<E>(e) << std::endl;
    }
};

template<typename T, typename A>
auto operator|(single_deferred<T>&& in, A&& a) {
    return a(std::move(in));
}
template<typename T, typename A>
auto operator|(const single_deferred<T>& in, A&& a) {
    return a(in);
}
