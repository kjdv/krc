#include "single_executor.hh"
#include <cassert>
#include "debug.hh"

using namespace std;

namespace krc {

single_executor::single_executor()
{}

single_executor::~single_executor()
{
    cleanup();
}


void single_executor::dispatch(target_t target)
{
    assert(target.stack_size >= MIN_STACK_SIZE && "stack size too small");

    auto fn = move(target.target);
    auto wrapped = [=] {
        fn();
        this->next();

        assert(false && "unreachable");
    };

    auto handle = context<>::make(target_t(wrapped, target.stack_size));
    d_schedule.push(handle);
}

void single_executor::run(target_t target)
{
    assert(d_main == nullptr && "run() called while already running");

    dispatch(move(target));

    d_main = context<>::main();
    context<>::swap(d_main, d_schedule.front());

    // cleanup (all this can't throw, so no need to get smart about exception safety and raii)
    cleanup();
}

void single_executor::yield()
{
    debug("before yield " + std::to_string(d_schedule.size()) + " " + to_string((uintptr_t)this));

    gc();

    if(d_schedule.size() > 1)
    {
        d_schedule.push(d_schedule.front());
        d_schedule.pop();

        context<>::swap(d_schedule.back(), d_schedule.front());
    }


    debug("after yield " + std::to_string(d_schedule.size()));
}

routine_id single_executor::get_id()
{
    return context<>::get_id();
}

void single_executor::next()
{
    debug("checking schedule " + to_string((uintptr_t)this));
    assert(!d_schedule.empty());

    gc();
    d_garbage.push_back(d_schedule.front());
    d_schedule.pop();

    if(d_schedule.empty()) // nothing else to do, return to main
        context<>::set(d_main);
    else
        context<>::set(d_schedule.front());
}

void single_executor::cleanup()
{
    debug("cleaning up " + to_string((uintptr_t)this));

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

void single_executor::gc()
{
    for(auto h : d_garbage)
        context<>::release(h);
    d_garbage.clear();
}




}
