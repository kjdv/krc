#include "executor.hh"
#include <cassert>

using namespace std;

namespace krc {
namespace {

// the purists will hate this use of singletons, but for this domain with
// stacks changing underneath you and the (single) executor having by its
// nature an effect on a global or thread scope it seems like the right thing
thread_local single_executor *t_exec{nullptr};

struct defer
{
    std::function<void()> func;
    ~defer()
    {
        func();
    }
};

}

executor executor::s_instance;

executor& executor::instance()
{
    return s_instance;
}

void executor::dispatch(const target_t &target)
{
    assert(d_dispatcher && "called from outside a krc execution scope");
    d_dispatcher(target);
}

void executor::run(const target_t &target, size_t num_threads)
{
    assert(t_exec == nullptr && "run() called more than once");

    run_single(target);
}

void executor::run_single(const target_t &target)
{
    single_executor se;
    t_exec = &se;
    d_dispatcher = [](const target_t &item) {
        assert(t_exec != nullptr);
        t_exec->dispatch(item);
    };

    defer cleanup{[this] {
            d_dispatcher = std::function<void(target_t)>();
            t_exec = nullptr;
    }};

    se.run(target);
}

void executor::yield()
{
    if (t_exec != nullptr)
        t_exec->yield();
    else
        this_thread::yield();
}

routine_id executor::get_id()
{
    return t_exec ? t_exec->get_id() : no_routine_id;
}

executor::executor()
{}

} // namespace krc
