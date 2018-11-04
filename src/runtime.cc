#include "runtime.hh"
#include "executor.hh"

namespace krc {

void dispatch(const target_t &target)
{
    auto& exec = executor::instance();
    exec.dispatch(target);
}

void run(const target_t &target)
{
    auto& exec = executor::instance();
    exec.run(target);
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
