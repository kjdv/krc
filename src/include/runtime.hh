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

    callable_t target;
    size_t stack_size{0};

    target_t() = default;
    ~target_t() = default;

    target_t(const target_t &other)
        : target(other.target)
        , stack_size(other.stack_size)
    {}

    target_t(target_t &&other)
        : target(std::move(other.target))
        , stack_size(other.stack_size)
    {}

    template <typename F>
    target_t(F&& target_, size_t stack_size_ = DEFAULT_STACK_SIZE)
        : target(target_)
        , stack_size(stack_size_)
    {}

    target_t &operator=(const target_t &other)
    {
        if (this != &other)
        {
            target = other.target;
            stack_size = other.stack_size;
        }
        return *this;
    }

    target_t &operator=(target_t &&other)
    {
        if (this != &other)
        {
            target = std::move(other.target);
            stack_size = other.stack_size;
        }
        return *this;
    }
};

typedef uintptr_t routine_id;
constexpr routine_id no_routine_id{0};

// runs the target function as a coroutine. This can only be called after run(), from within run().
void dispatch(const target_t &target);

// run the coroutines. You need to supply at least the top-level target
void run(const target_t &target, size_t num_threads = 1);

// returns true if control was yielded, false if this could not be done (no other routines waiting)
void yield();

routine_id get_id();

} // namespace krc
