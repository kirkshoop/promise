#include <iostream>
#include <string>

#include "single.h"
#include "lifetime.h"
#include "algorithms.h"

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

    return 0;
#endif
}