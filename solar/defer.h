#pragma once

// https://psbs.tistory.com/18
// 함수가 끝날때 같이 해제하기 defer

#include <functional>
#include <type_traits>

template<typename _Func>
struct Defer {
    using value_type = _Func;
    using const_reference = typename std::add_lvalue_reference<typename std::add_const<_Func>>::type;

    value_type defer_function;
    Defer(const_reference f) : defer_function(f) {}
    Defer(value_type&& f) : defer_function(f) {}
    ~Defer() { run_function(std::is_assignable<_Func, std::nullptr_t>()); }

    inline void run_function(std::false_type /*lambda function, const function pointer*/) {
        defer_function();
    }
    inline void run_function(std::true_type /*std::function<>, function pointer*/) {
        if (defer_function) {
            defer_function();
        }
    }
};

template<typename _Func>
auto make_defer(_Func&& f) -> Defer<_Func> {
    return Defer<_Func>(std::forward<_Func>(f));
}
template<typename _Func>
auto make_defer(typename std::add_lvalue_reference<typename std::add_const<_Func>>::type f) -> Defer<_Func> {
    return Defer<_Func>(std::forward<_Func>(f));
}

#define MAKE_TOKEN_PASTE(x, y)  x##y
#define MAKE_TOKEN(x, y)        MAKE_TOKEN_PASTE(x, y)
#define defer(__CODE__BLOCK__)  auto MAKE_TOKEN(_temp_defer,__LINE__){make_defer([=]{__CODE__BLOCK__;})}
#define deferf(__CODE__BLOCK__) auto MAKE_TOKEN(_temp_defer,__LINE__){make_defer(__CODE__BLOCK__)}
