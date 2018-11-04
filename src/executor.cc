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
    d_exec.dispatch(target, stack_size);
}

void executor::run(const std::function<void()>& target, size_t stack_size)
{
    d_exec.run(target, stack_size);
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
