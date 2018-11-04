#include "internal/mutex.hh"
#include "executor.hh"

namespace krc {
namespace internal {

namespace {

constexpr std::atomic_flag init_flag()
{
    return ATOMIC_FLAG_INIT;
}

} // namespace

mutex::mutex()
    : d_lock(init_flag())
{}

void mutex::lock()
{
    auto& exec = executor::instance();

    while(!try_lock())
        exec.yield();
}

void mutex::unlock()
{
    d_lock.clear(std::memory_order_release);

    auto& exec = executor::instance();
    exec.yield(); // this could well unblock someone else
}

bool mutex::try_lock()
{
    return d_lock.test_and_set(std::memory_order_acquire) == false;
}

} // namespace internal
} // namespace krc
