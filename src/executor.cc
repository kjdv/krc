#include "executor.hh"
#include <cassert>

using namespace std;

namespace krc {

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
        d_waiting.push(d_current);
        d_current = d_waiting.front();
        d_waiting.pop();

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
