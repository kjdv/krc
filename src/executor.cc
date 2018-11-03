#include "executor.hh"
#include <cassert>

using namespace std;

namespace krc {

executor executor::s_instance;

executor& executor::instance()
{
    return s_instance;
}

void executor::dispatch(const std::function<void()>& target, size_t stack_size)
{
    assert(stack_size >= MIN_STACK_SIZE && "stack size too small");

    auto wrapped = [=] {
        target();
        this->next();

        assert(false && "unreachable");
    };

    auto handle = context<>::make(wrapped, stack_size);
    d_schedule.push(handle);
}

void executor::run(const std::function<void()>& target, size_t stack_size)
{
    assert(d_main == nullptr && "run() called while already running");

    dispatch(target, stack_size);

    d_main = context<>::main();
    context<>::swap(d_main, d_schedule.front());

    // cleanup (all this can't throw, so no need to get smart about exception safety and raii)
    cleanup();
}

void executor::yield()
{
    gc();

    if(d_schedule.size() > 1)
    {
        d_schedule.push(d_schedule.front());
        d_schedule.pop();

        context<>::swap(d_schedule.back(), d_schedule.front());
    }
}

routine_id executor::get_id()
{
    return context<>::get_id();
}

executor::executor()
{
}

executor::~executor()
{
    cleanup();
}

void executor::next()
{
    assert(!d_schedule.empty());

    gc();
    d_garbage.push_back(d_schedule.front());
    d_schedule.pop();

    if(d_schedule.empty()) // nothing else to do, return to main
        context<>::set(d_main);
    else
        context<>::set(d_schedule.front());
}

void executor::cleanup()
{
    gc();

    while(!d_schedule.empty())
    {
        auto h = d_schedule.front();
        d_schedule.pop();
        context<>::release(h);
    }
    context<>::release(d_main);

    d_main = nullptr;
}

void executor::gc()
{
    for(auto h : d_garbage)
        context<>::release(h);
    d_garbage.clear();
}

} // namespace krc
