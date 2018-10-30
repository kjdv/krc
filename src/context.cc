#include <context.hh>
#include <ucontext.h>
#include <cstdlib>
#include <csignal>
#include <cstdarg>
#include <cassert>

namespace krc {
namespace {

struct ucontext_handle
{
    ucontext_t ctx;
    target_t target;
    void *stack_ptr;
};


enum { offset = sizeof(ucontext_handle) };

}

}

extern "C" void krc_run_target(void *vp)
{
    using namespace krc;

    ucontext_handle *handle = reinterpret_cast<ucontext_handle *>(vp);

    assert((char *)handle->stack_ptr + krc::offset == handle->ctx.uc_stack.ss_sp);

    handle->target();

    // clean up ourselves
    void *sp = handle->stack_ptr;
    handle->~ucontext_handle();

    // last bit, free the stack
    free(sp);
}

namespace krc {
context<context_method::UCONTEXT>::handle make(const target_t &target, size_t stack_size)
{
    void *stack = malloc(stack_size + offset);
    ucontext_handle *handle = new(stack) ucontext_handle{ucontext_t{}, target, stack};

    getcontext(&handle->ctx);
    handle->ctx.uc_stack.ss_sp = (char *)handle->stack_ptr + offset;
    handle->ctx.uc_stack.ss_size = stack_size;

    sigemptyset(&handle->ctx.uc_sigmask); // todo: figure out the SIGFPE issue

    makecontext(&handle->ctx, (void(*)()) &krc_run_target, 1, handle);

    return handle;
}

void context<context_method::UCONTEXT>::swap(handle old_ctx, handle new_ctx)
{
    swapcontext(&reinterpret_cast<ucontext_handle *>(old_ctx)->ctx,
                &reinterpret_cast<ucontext_handle *>(new_ctx)->ctx);
}

}
