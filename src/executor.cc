#include "executor.hh"
#include <cassert>

using namespace std;

namespace krc {

// weird control ping-poing needed because makecontext targets need c-linkage
std::function<void(int)> g_execute;
void                     run_target(int routine_id)
{
    g_execute(routine_id);
}

executor executor::s_instance;

executor& executor::instance()
{
    return s_instance;
}

executor::executor()
{
    g_execute = [=](int routine_id) { this->execute(routine_id); };
}

void executor::dispatch(const function<void()>& target, size_t stack_size)
{
    assert(stack_size >= MIN_STACK_SIZE);

    int  routine_id = d_targets.empty() ? 0 : d_targets.rbegin()->first + 1;
    auto it         = d_targets.emplace(std::piecewise_construct,
                                std::forward_as_tuple(routine_id),
                                std::forward_as_tuple(target, stack_size));
    assert(it.second);

    ucontext_t ctx;
    getcontext(&ctx);
    ctx.uc_stack.ss_sp    = it.first->second.stack.data();
    ctx.uc_stack.ss_size  = stack_size;
    ctx.uc_stack.ss_flags = 0;
    ctx.uc_link           = 0;
    sigemptyset(&ctx.uc_sigmask); // todo: figure out the SIGFPE issue
    // sigdelset(&ctx.uc_sigmask, SIGFPE);
    makecontext(&ctx, (void (*)()) & run_target, 1, routine_id);

    d_routines.push(ctx);
}

void executor::execute(int routine_id)
{
    auto it = d_targets.find(routine_id);
    assert(it != d_targets.end());

    it->second.target();

    // cleanup
    d_targets.erase(it);

    next();
}

void executor::run()
{
    if(!d_routines.empty())
    {
        ucontext_t ctx = d_routines.front();
        d_routines.pop();

        swapcontext(&d_main, &ctx);
    }
}

void executor::run(const function<void()>& target, size_t stack_size)
{
    dispatch(target, stack_size);
    run();
}

bool executor::yield()
{
    if(!d_routines.empty())
    {
        d_routines.emplace();

        ucontext_t new_ctx = d_routines.front();
        d_routines.pop();

        swapcontext(&d_routines.back(), &new_ctx);

        return true;
    }
    return false;
}

void executor::next()
{
    if(d_routines.empty())
    {
        setcontext(&d_main);
    }
    else
    {
        ucontext_t ctx = d_routines.front();
        d_routines.pop();

        setcontext(&ctx);
    }
}

executor::target_t::target_t(const std::function<void()>& target_, size_t stack_size)
    : target(target_)
    , stack(stack_size)
{
}

} // namespace krc
