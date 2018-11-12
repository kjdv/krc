#include "internal/semaphore.hh"
#include "executor.hh"

using namespace std;

namespace krc {
namespace internal {

semaphore::semaphore(int count)
    : d_count(count)
{}

void semaphore::wait()
{
    auto& exec = executor::instance();
    while(!try_wait())
        exec.yield();
}

bool semaphore::try_wait()
{
    int count = d_count;
    if(count)
        return d_count.compare_exchange_strong(count, count - 1);
    else
        return false;
}

void semaphore::notify()
{
    ++d_count;

    auto& exec = executor::instance();
    exec.yield();
}

}
}
