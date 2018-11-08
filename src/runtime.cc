#include "runtime.hh"
#include "executor.hh"

namespace krc {

void dispatch(target_t target)
{
    auto& exec = executor::instance();
    exec.dispatch(std::move(target));
}

void run(target_t target, size_t num_threads)
{
    auto& exec = executor::instance();
    exec.run(std::move(target), num_threads);
}

void yield()
{
    auto& exec = executor::instance();
    exec.yield();
}

routine_id get_id()
{
    auto& exec = executor::instance();
    return exec.get_id();
}

} // namespace krc
