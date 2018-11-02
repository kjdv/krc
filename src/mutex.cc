#include "mutex.hh"
#include "executor.hh"

namespace krc {

void mutex::lock()
{
    auto& exec = executor::instance();

    bool expect = false;
    while(!d_held.compare_exchange_weak(expect, true))
    {
        expect = false;
        exec.yield();
    }
}

void mutex::unlock()
{
    d_held.store(false);

    auto& exec = executor::instance();
    exec.yield(); // this could well unblock someone else
}

bool mutex::try_lock()
{
    bool expect = false;
    return d_held.compare_exchange_strong(expect, true);
}

} // namespace krc
