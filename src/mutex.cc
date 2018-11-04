#include "internal/mutex.hh"
#include "executor.hh"

namespace krc {
namespace internal {

spinlock::spinlock()
    : d_lock ATOMIC_FLAG_INIT
{}

void spinlock::lock()
{
    while(!try_lock())
        ;
}

void spinlock::unlock()
{
    d_lock.clear(std::memory_order_release);
}

bool spinlock::try_lock()
{
    return d_lock.test_and_set(std::memory_order_acquire) == false;
}

void mutex::lock()
{
    auto& exec = executor::instance();

    while(!try_lock())
        exec.yield();
}

void mutex::unlock()
{
    d_lock.unlock();

    auto& exec = executor::instance();
    exec.yield(); // this could well unblock someone else
}

bool mutex::try_lock()
{
    return d_lock.try_lock();
}

} // namespace internal
} // namespace krc
