#include "internal/semaphore.hh"
#include "executor.hh"
#include "debug.hh"

using namespace std;

namespace krc {
namespace internal {

binary_semaphore::binary_semaphore(bool init)
    : d_notified(init)
{}

void binary_semaphore::wait()
{
    auto& exec = executor::instance();

    while (!try_wait())
        exec.yield();
}

void binary_semaphore::notify()
{
    constexpr char c = 1;
    atomic_fetch_or(&d_notified, c);

    auto& exec = executor::instance();
    exec.yield();
}

bool binary_semaphore::try_wait()
{
    constexpr char c = 0;
    return atomic_fetch_and(&d_notified, c);
}

closeable_binary_semaphore::closeable_binary_semaphore(bool init)
    : d_sem(init)
{}

void closeable_binary_semaphore::wait()
{
    auto& exec = executor::instance();

    while (!d_closed && !d_sem.try_wait())
        exec.yield();
}

void closeable_binary_semaphore::notify()
{
    d_sem.notify();
}

void closeable_binary_semaphore::close()
{
    d_closed.store(true);
}

bool closeable_binary_semaphore::is_closed() const
{
    return d_closed;
}

}
}
