#include "executor.hh"
#include <cassert>

using namespace std;

namespace krc {

// weird control ping-poing needed because makecontext targets need c-linkage
std::function<void(int)> g_execute;

void run_target(int routine_id)
{
    g_execute(routine_id);
}

executor_old executor_old::s_instance;

executor_old& executor_old::instance()
{
    return s_instance;
}

executor_old::executor_old()
{
    g_execute = [=](int routine_id) { this->execute(routine_id); };
}

void executor_old::dispatch(const function<void()>& target, size_t stack_size)
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

void executor_old::execute(int routine_id)
{
    auto it = d_targets.find(routine_id);
    assert(it != d_targets.end());

    it->second.target();

    // cleanup
    d_targets.erase(it);

    next();
}

void executor_old::run()
{
    if(!d_routines.empty())
    {
        ucontext_t ctx = d_routines.front();
        d_routines.pop();

        swapcontext(&d_main, &ctx);
    }
}

void executor_old::run(const function<void()>& target, size_t stack_size)
{
    dispatch(target, stack_size);
    run();
}

bool executor_old::yield()
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

void executor_old::next()
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

executor_old::target_t::target_t(const std::function<void()>& target_, size_t stack_size)
    : target(target_)
    , stack(stack_size)
{
}

executor executor::s_instance;

executor &executor::instance()
{
    return s_instance;
}

void executor::dispatch(const std::function<void ()> &target, size_t stack_size)
{
    auto wrapped = [=]{
        target();
        this->next();
    };

    auto handle = context<>::make(wrapped, stack_size);
    d_waiting.push(handle);
}

void executor::run(const std::function<void ()> &target, size_t stack_size)
{
    assert(d_main == nullptr && "run() called while already running");
    assert(d_current == nullptr);

    dispatch(target, stack_size);

    d_main = context<>::main();
    d_current = d_waiting.front();
    d_waiting.pop();
    context<>::swap(d_main, d_current);

    // cleanup (all this can't throw, so no need to get smart about exception safety and raii)
    cleanup();
}

void executor::yield()
{
    gc();

    if (!d_waiting.empty())
    {
        auto new_ctx = d_waiting.front();
        d_waiting.pop();

        d_waiting.push(d_current);
        d_current = new_ctx;
        context<>::swap(d_waiting.back(), d_current);
    }
}

routine_id executor::get_id()
{
    return context<>::get_id();
}

executor::executor()
{}

executor::~executor()
{
    cleanup();
}

void executor::next()
{
    gc();
    d_garbage.push_back(d_current);

    if (d_waiting.empty()) // nothing else to do, return to main
        context<>::set(d_main);
    else
    {
        d_current = d_waiting.front();
        d_waiting.pop();
        context<>::set(d_current);
    }
}

void executor::cleanup()
{
    gc();

    while(!d_waiting.empty())
    {
        auto h = d_waiting.front();
        d_waiting.pop();
        context<>::release(h);
    }
    context<>::release(d_main);

    d_current = nullptr; // released via gc()
    d_main = nullptr;
}

void executor::gc()
{
    for (auto h : d_garbage)
        context<>::release(h);
    d_garbage.clear();
}

} // namespace krc
