#include "mutex.hh"
#include "executor.hh"

namespace krc {

mutex::mutex()
{}

void mutex::lock()
{
    // todo: this could be an opportunity to auto-detect deadlocks, throw an exception if no other active routines are found

    auto &exec = executor::instance();
    while (!d_base.try_lock())
        exec.yield();
}

void mutex::unlock()
{
    d_base.unlock();
}

bool mutex::try_lock()
{
    return d_base.try_lock();
}

}
