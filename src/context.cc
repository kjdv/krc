#include <context.hh>
#include <cstdlib>
#include <csignal>
#include <cassert>
#include <cerrno>
#include <iostream>
#include <cstring>
#include <ucontext.h>

#include "ptr_translate.hh"

extern "C" void krc_run_target(int p1, int p2);

namespace krc {
namespace {


struct ucontext_handle
{
    ucontext_t ctx;
    target_t target;
    char *stack_ptr{nullptr};
};


enum { offset = sizeof(ucontext_handle) };

ucontext_handle g_main;

thread_local context<context_method::UCONTEXT>::id g_current_id{context<context_method::UCONTEXT>::no_context};

void set_id(const ucontext_handle &h)
{
    g_current_id = reinterpret_cast<context<context_method::UCONTEXT>::id>(h.stack_ptr);
}

}

context<context_method::UCONTEXT>::handle context<context_method::UCONTEXT>::make(const target_t &target, size_t stack_size)
{
    assert(stack_size >= MINSIGSTKSZ && "stack size too small");

    char *stack = new char[stack_size + offset];
    ucontext_handle *handle = new(stack) ucontext_handle{ucontext_t{}, target, stack};

    getcontext(&handle->ctx);
    handle->ctx.uc_stack.ss_sp = handle->stack_ptr + offset;
    handle->ctx.uc_stack.ss_size = stack_size;

    sigemptyset(&handle->ctx.uc_sigmask); // todo: figure out the SIGFPE issue

    // magic to translate a void* into two ints
    int p1, p2;
    from_ptr(handle, p1, p2);

    makecontext(&handle->ctx, (void(*)()) krc_run_target, 2, p1, p2);

    return handle;
}

void context<context_method::UCONTEXT>::swap(handle old_ctx, handle new_ctx)
{
    ucontext_handle *o = reinterpret_cast<ucontext_handle *>(old_ctx);
    ucontext_handle *n = reinterpret_cast<ucontext_handle *>(new_ctx);

    set_id(*n);
    int rc = swapcontext(&o->ctx, &n->ctx);

    if(rc != 0)
        std::cerr << strerror(errno) << std::endl;
    assert(rc == 0 && "swapcontext failed");
}

context<context_method::UCONTEXT>::handle context<context_method::UCONTEXT>::main()
{
    assert(g_current_id == no_context && "main() called from within an active context");

    getcontext(&g_main.ctx);
    return &g_main;
}

void context<context_method::UCONTEXT>::release(handle h)
{
    ucontext_handle *handle = reinterpret_cast<ucontext_handle *>(h);

    if (handle && handle != &g_main)
    {
        // clean up ourselves
        handle->~ucontext_handle();

        // last bit, free the stack
        delete [] reinterpret_cast<char *>(handle);
    }
}

context<context_method::UCONTEXT>::id context<context_method::UCONTEXT>::get_id()
{
    return g_current_id;
}

}

void krc_run_target(int p1, int p2)
{
    using namespace krc;

    auto hp = to_ptr(p1, p2);
    ucontext_handle *handle = reinterpret_cast<ucontext_handle *>(hp);

    assert((char *)handle->stack_ptr + krc::offset == handle->ctx.uc_stack.ss_sp);

    handle->target();

    assert(false && "routine ended before handing back control to main");
}
