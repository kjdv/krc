#include "executor.hh"
#include <cassert>

using namespace std;

namespace krc {

executor executor::s_instance;

executor& executor::instance()
{
    return s_instance;
}

void executor::dispatch(const target_t &target)
{
    d_exec.dispatch(target);
}

void executor::run(const target_t &target)
{
    d_exec.run(target);
}

void executor::yield()
{
    d_exec.yield();
}

routine_id executor::get_id()
{
    return d_exec.get_id();
}

executor::executor()
{}

} // namespace krc
