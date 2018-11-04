#pragma once

#include <algorithm>
#include <functional>
#include <signal.h>

namespace krc {

enum {
    MIN_STACK_SIZE     = MINSIGSTKSZ,
    DEFAULT_STACK_SIZE = std::max(1 << 16, MIN_STACK_SIZE),
};

struct target_t
{
    typedef std::function<void()> callable_t;

    const callable_t target;
    const size_t stack_size{0};

    target_t()
    {}
    ~target_t() = default;

    template <typename F>
    target_t(F&& target_, size_t stack_size_ = DEFAULT_STACK_SIZE)
        : target(target_)
        , stack_size(stack_size_)
    {}
};

typedef uintptr_t routine_id;

// runs the target function as a coroutine. This can be called before or after a call to start()
void dispatch(const target_t &target);

// run the coroutines. You need to supply at least the top-level target
void run(const target_t &target);

// returns true if control was yielded, false if this could not be done (no other routines waiting)
void yield();

routine_id get_id();

} // namespace krc
