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

template <>
struct context<context_method::UCONTEXT>
{
    typedef void *handle;

    static handle make(const target_t &target, size_t stack_size);

    static void swap(handle old_ctx, handle new_ctx);

    static handle main();

    static void release(handle h);
};

}
