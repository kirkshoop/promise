#include <iostream>
#include <string>

#include "single.h"
#include "lifetime.h"
#include "algorithms.h"

#include "promise_1.h"
#include "promise_2.h"

#include "packaged_task.h"

int main() {

    auto int_answer = single_create([](auto&& s){
        std::this_thread::sleep_for(1s);
        s.value(42);
    });

#if 1
    int_answer | 
        single_subscribe(single<single_ostream>{single_ostream{std::cout}});
#endif

    auto string_answer = int_answer | 
        single_transform([](int i){ return std::to_string(i); }) |
        single_produce_on(std::launch::async);

#if 1
    auto l = string_answer | 
        single_subscribe(single<single_ostream>{single_ostream{std::cout}});

    l.stop();
#endif

#if 0
    string_answer | 
        single_subscribe(single<single_ostream>{single_ostream{std::cout}});

    std::this_thread::sleep_for(2s);
#endif

#if 1
    string_answer | 
        single_tap(single<single_ostream>{single_ostream{std::cout}}) |
        single_wait();

#endif

#if 1
    {
        using namespace promise_1;

        promise<int> p{[](auto v, auto ){
            std::this_thread::sleep_for(1s);
            v(42);
        }};

        p
            .then([](int v){ return std::to_string(v); })
            .then([](std::string s){ std::cout << s << std::endl; return 0; })
            .wait();
    }
#endif

#if 1
    {
        using namespace promise_2;

        promise<int> p;
        
        std::thread t([p]() mutable {
            std::this_thread::sleep_for(1s);
            p.set_value(42);
        });

        p.get_future()
            .then([](int v){ return std::to_string(v); })
            .then([](std::string s){ std::cout << s << std::endl; return 0; })
            .wait();

        t.join();
    }
#endif

    {
        using namespace packaged_task;

        packaged_task<int()> p{[]() {
            std::this_thread::sleep_for(1s);
            return 42;
        }};

        auto f = p.get_future();

        std::thread t(std::move(p));

        f
            .then([](int v){ return std::to_string(v); })
            .then([](std::string s){ std::cout << s << std::endl; return 0; })
            .wait();

        t.join();
    }

    return 0;
}