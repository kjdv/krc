#include "internal/condition_variable.hh"
#include "executor.hh"

namespace krc {
namespace internal {

void condition_variable::wait(lock_t &l)
{
    auto &exec = executor::instance();

    l.unlock();
    exec.yield();
    l.lock();
}

void condition_variable::wait(lock_t &l, const predicate_t &p)
{
    while(!p())
        wait(l);
}

}
}
