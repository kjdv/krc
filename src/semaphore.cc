#include "internal/semaphore.hh"
#include "executor.hh"

using namespace std;

namespace krc {
namespace internal {

binary_semaphore::binary_semaphore(bool init)
    : d_notified(init)
{}

void binary_semaphore::wait()
{
    auto& exec = executor::instance();

    constexpr char c = 0;
    while (!atomic_fetch_and(&d_notified, c))
        exec.yield();
}

void binary_semaphore::notify()
{
    constexpr char c = 1;
    atomic_fetch_or(&d_notified, c);

    auto& exec = executor::instance();
    exec.yield();
}

}
}
