#pragma once

#include <functional>

namespace krc {

typedef std::function<void()> target_t;

enum class context_method {
    UCONTEXT,
    DEFAULT = context_method::UCONTEXT
};

template <context_method M = context_method::DEFAULT>
struct context;

// wrapper around low-level context manipulation
template <>
struct context<context_method::UCONTEXT>
{
    typedef void *handle;
    typedef uintptr_t id;
    constexpr static id no_context = 0;

    // creates a new handle
    static handle make(const target_t &target, size_t stack_size);

    // yields control to new_ctx, while storing the current context in old_ctx
    static void swap(handle old_ctx, handle new_ctx);

    // set the new context
    static void set(handle new_ctx);

    // create a handle to the main context. it is ub to call this when there is an active context
    static handle main();

    // free resources associated with h
    static void release(handle h);

    // get the id of the current running context
    static id get_id();
};

}
