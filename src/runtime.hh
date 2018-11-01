#pragma once

#include <functional>
#include <signal.h>

namespace krc {

enum {
    DEFAULT_STACK_SIZE = 1 << 15,
    MIN_STACK_SIZE     = MINSIGSTKSZ,
};

static_assert(MIN_STACK_SIZE <= DEFAULT_STACK_SIZE);

typedef uintptr_t routine_id;

// runs the target function as a coroutine. This can be called before or after a call to start()
void dispatch(const std::function<void()>& target, size_t stack_size = DEFAULT_STACK_SIZE);

// run the coroutines. You need to supply at least the top-level target
void run(const std::function<void()>& target, size_t stack_size = DEFAULT_STACK_SIZE);

// returns true if control was yielded, false if this could not be done (no other routines waiting)
void yield();

routine_id get_id();

} // namespace krc
