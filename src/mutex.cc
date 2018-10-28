#include "mutex.hh"
#include "executor.hh"

namespace krc {

mutex::mutex()
{}

void mutex::lock()
{
    // todo: this could be an opportunity to auto-detect deadlocks, throw an exception if no other active routines are found

    auto &exec = executor::instance();
    while (!try_lock())
        exec.yield();
}

void mutex::unlock()
{
    std::lock_guard<std::mutex> l(d_base);
    d_held = false;
}

bool mutex::try_lock()
{
    std::lock_guard<std::mutex> l(d_base);
    if(d_held)
        return false;

    d_held = true;
    return true;
}

}
